#!/usr/bin/env python3
import argparse
import configparser
import datetime
import json
import re
import struct
import sys
import urllib.error
import urllib.request
import uuid
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple

OSV_BATCH_URL = "https://api.osv.dev/v1/querybatch"
OSV_VULN_URL = "https://api.osv.dev/v1/vulns/{}"
MCUBOOT_MAGIC = 0x96F3B83D

BANNED_C_SINKS = {
    "strcpy":  ("CWE-120", "HIGH",     "Unbounded string copy; replace with snprintf() or bounded memcpy()"),
    "sprintf": ("CWE-120", "HIGH",     "Unbounded format write; replace with snprintf()"),
    "strcat":  ("CWE-120", "HIGH",     "Unbounded string concat; replace with strncat() or snprintf()"),
    "gets":    ("CWE-242", "CRITICAL", "Inherently unsafe input read; replace with fgets()"),
}

SECRET_REGEXES = [
    (re.compile(r'(?i)(password|passwd|mqtt_pass|api_key|wifi_pass)\s*=\s*"([^"]{6,})"'),
     "CWE-798", "CRITICAL", "Hardcoded plaintext credential in source"),
    (re.compile(r'-----BEGIN (RSA |EC |OPENSSH )?PRIVATE KEY-----'),
     "CWE-321", "CRITICAL", "Embedded private cryptographic key in firmware tree"),
]

@dataclass
class ComponentDep:
    name: str
    version: str
    ecosystem: str
    source_file: str
    repo_url: Optional[str] = None
    commit_sha: Optional[str] = None
    purl: str = ""
    osv_vulns: List[Dict[str, str]] = field(default_factory=list)

    def build_purl(self) -> str:
        if self.purl:
            return self.purl
        ver = self.commit_sha or self.version or "unknown"
        if self.ecosystem == "PyPI":
            return f"pkg:pypi/{self.name.lower()}@{ver}"
        if self.ecosystem == "npm":
            return f"pkg:npm/{self.name}@{ver}"
        if self.repo_url and "github.com/" in self.repo_url:
            slug = self.repo_url.split("github.com/", 1)[1].rstrip("/").removesuffix(".git")
            return f"pkg:github/{slug}@{ver}"
        return f"pkg:generic/{self.name}@{ver}"

    def to_osv_query(self) -> Optional[Dict]:
        if self.commit_sha and re.fullmatch(r"[0-9a-fA-F]{40}", self.commit_sha):
            return {"commit": self.commit_sha}
        if self.ecosystem == "GIT" and self.repo_url and self.version:
            return {"package": {"ecosystem": "GIT", "name": self.repo_url}, "version": self.version}
        if self.ecosystem in {"PyPI", "npm"} and self.name and self.version:
            return {"package": {"ecosystem": self.ecosystem, "name": self.name}, "version": self.version}
        return None

def parse_platformio_ini(ini_path: Path, root: Path) -> List[ComponentDep]:
    deps: List[ComponentDep] = []
    rel = str(ini_path.relative_to(root))
    parser = configparser.ConfigParser(interpolation=None, strict=False)
    try:
        parser.read(ini_path, encoding="utf-8")
    except Exception:
        return deps
    seen = set()
    for section in parser.sections():
        if not parser.has_option(section, "lib_deps"):
            continue
        for line in parser.get(section, "lib_deps").splitlines():
            entry = line.strip().strip(",")
            if not entry or entry.startswith((";", "#", "${")):
                continue
            git_m = re.search(r'(https?://github\.com/[^\s#@]+?)(?:\.git)?(?:#([^\s]+))?$', entry)
            if git_m:
                repo_url = git_m.group(1).rstrip("/")
                ref = (git_m.group(2) or "").strip()
                name = repo_url.split("/")[-1]
                is_sha = bool(re.fullmatch(r"[0-9a-fA-F]{40}", ref))
                key = (repo_url, ref)
                if key not in seen:
                    seen.add(key)
                    deps.append(ComponentDep(name, "" if is_sha else ref, "GIT", rel, repo_url, ref if is_sha else None))
                continue
            pio_m = re.match(r'^([A-Za-z0-9_.\-/]+)\s*@\s*[\^~>=<]*([0-9][A-Za-z0-9_.\-]+)', entry)
            if pio_m:
                name, ver = pio_m.group(1).strip(), pio_m.group(2).strip()
                key = (name, ver)
                if key not in seen:
                    seen.add(key)
                    repo_url = f"https://github.com/{name}" if "/" in name else None
                    deps.append(ComponentDep(name, ver, "GIT" if repo_url else "PlatformIO", rel, repo_url))
    return deps

def parse_cmake_lists(cmake_path: Path, root: Path) -> List[ComponentDep]:
    deps: List[ComponentDep] = []
    rel = str(cmake_path.relative_to(root))
    text = cmake_path.read_text(encoding="utf-8", errors="ignore")
    for block in re.findall(r'FetchContent_Declare\s*\((.*?)\)', text, flags=re.S | re.I):
        url_m = re.search(r'GIT_REPOSITORY\s+([^\s\)]+)', block)
        tag_m = re.search(r'GIT_TAG\s+([^\s\)]+)', block)
        if url_m:
            repo_url = url_m.group(1).strip().strip('"').removesuffix(".git")
            ref = tag_m.group(1).strip().strip('"') if tag_m else ""
            is_sha = bool(re.fullmatch(r"[0-9a-fA-F]{40}", ref))
            deps.append(ComponentDep(repo_url.split("/")[-1], "" if is_sha else ref, "GIT", rel, repo_url, ref if is_sha else None))
    return deps

def parse_requirements_txt(req_path: Path, root: Path) -> List[ComponentDep]:
    deps: List[ComponentDep] = []
    rel = str(req_path.relative_to(root))
    for line in req_path.read_text(encoding="utf-8", errors="ignore").splitlines():
        line = line.split("#", 1)[0].strip()
        m = re.match(r'^([A-Za-z0-9_.\-]+)==([A-Za-z0-9_.\-+]+)', line)
        if m:
            deps.append(ComponentDep(m.group(1), m.group(2), "PyPI", rel))
    return deps

def parse_package_json(pkg_path: Path, root: Path) -> List[ComponentDep]:
    deps: List[ComponentDep] = []
    rel = str(pkg_path.relative_to(root))
    try:
        data = json.loads(pkg_path.read_text(encoding="utf-8", errors="ignore"))
    except Exception:
        return deps
    for section in ("dependencies", "devDependencies"):
        for name, raw_ver in (data.get(section) or {}).items():
            ver = re.sub(r'^[\^~>=<\s]+', '', str(raw_ver)).strip()
            if re.match(r'^[0-9]+\.[0-9]+', ver):
                deps.append(ComponentDep(name, ver, "npm", rel))
    return deps

def fetch_osv_summary(vuln_id: str, timeout_sec: int = 8) -> str:
    req = urllib.request.Request(OSV_VULN_URL.format(vuln_id), headers={"User-Agent": "CRA-FirmAudit/2.0"})
    try:
        with urllib.request.urlopen(req, timeout=timeout_sec) as resp:
            data = json.loads(resp.read().decode("utf-8"))
            return (data.get("summary") or data.get("details") or "See OSV advisory")[:120].replace("\n", " ")
    except Exception:
        return f"https://osv.dev/vulnerability/{vuln_id}"

def query_osv_batch(deps: List[ComponentDep], timeout_sec: int = 15) -> Tuple[int, str]:
    indexed_queries = [(idx, q) for idx, d in enumerate(deps) if (q := d.to_osv_query())]
    if not indexed_queries:
        return 0, "No OSV-queryable dependencies found"
    payload = json.dumps({"queries": [q for _, q in indexed_queries]}).encode("utf-8")
    req = urllib.request.Request(OSV_BATCH_URL, data=payload, headers={"Content-Type": "application/json", "User-Agent": "CRA-FirmAudit/2.0"}, method="POST")
    try:
        with urllib.request.urlopen(req, timeout=timeout_sec) as resp:
            body = json.loads(resp.read().decode("utf-8"))
    except (urllib.error.URLError, TimeoutError, OSError) as exc:
        return 0, f"OSV.dev API unreachable ({exc})"
    total_vulns = 0
    for (dep_idx, _), res in zip(indexed_queries, body.get("results", [])):
        for v in (res.get("vulns") or []):
            vid = v.get("id", "UNKNOWN")
            deps[dep_idx].osv_vulns.append({"id": vid, "modified": v.get("modified", ""), "summary": fetch_osv_summary(vid)})
            total_vulns += 1
    return total_vulns, "OK"

def scan_c_sources_and_bins(root: Path) -> Tuple[int, List[Dict], List[Dict]]:
    c_files, sast_findings, bin_checks = 0, [], []
    for path in root.rglob("*"):
        if ".git" in path.parts or not path.is_file():
            continue
        suf, rel = path.suffix.lower(), str(path.relative_to(root))
        if suf in {".c", ".cpp", ".h", ".hpp", ".ino"}:
            c_files += 1
            for ln, line in enumerate(path.read_text(encoding="utf-8", errors="ignore").splitlines(), start=1):
                for func, (cwe, sev, fix) in BANNED_C_SINKS.items():
                    if re.search(rf'\b{func}\s*\(', line):
                        sast_findings.append({"file": rel, "line": ln, "cwe": cwe, "severity": sev, "sink": f"{func}()", "fix": fix})
                for rx, cwe, sev, fix in SECRET_REGEXES:
                    if rx.search(line):
                        sast_findings.append({"file": rel, "line": ln, "cwe": cwe, "severity": sev, "sink": "Hardcoded Secret", "fix": fix})
        elif suf == ".bin" and path.stat().st_size >= 4:
            magic, = struct.unpack("<I", path.read_bytes()[:4])
            bin_checks.append({"file": rel, "mcuboot_signed": (magic == MCUBOOT_MAGIC), "magic_hex": f"0x{magic:08X}"})
    return c_files, sast_findings, bin_checks

def build_cyclonedx_15(project_name: str, deps: List[ComponentDep]) -> Dict:
    components, vulnerabilities = [], []
    now_iso = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    for dep in deps:
        purl = dep.build_purl()
        bom_ref = f"comp-{uuid.uuid5(uuid.NAMESPACE_URL, purl)}"
        components.append({"bom-ref": bom_ref, "type": "library", "name": dep.name, "version": dep.commit_sha or dep.version or "unpinned", "purl": purl})
        for v in dep.osv_vulns:
            vulnerabilities.append({"id": v["id"], "source": {"name": "OSV.dev", "url": f"https://osv.dev/vulnerability/{v['id']}"}, "description": v["summary"], "affects": [{"ref": bom_ref}]})
    return {"bomFormat": "CycloneDX", "specVersion": "1.5", "serialNumber": f"urn:uuid:{uuid.uuid4()}", "version": 1, "metadata": {"timestamp": now_iso, "component": {"type": "firmware", "name": project_name}}, "components": components, "vulnerabilities": vulnerabilities}

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("target_dir")
    ap.add_argument("--sbom-out", default="bom.cdx.json")
    ap.add_argument("--report-out", default="CRA_AUDIT_REPORT.md")
    ap.add_argument("--offline", action="store_true")
    args = ap.parse_args()
    root = Path(args.target_dir).resolve()
    deps: List[ComponentDep] = []
    for path in root.rglob("*"):
        if ".git" in path.parts or not path.is_file():
            continue
        ln = path.name.lower()
        if ln == "platformio.ini": deps.extend(parse_platformio_ini(path, root))
        elif ln == "cmakelists.txt": deps.extend(parse_cmake_lists(path, root))
        elif ln == "requirements.txt": deps.extend(parse_requirements_txt(path, root))
        elif ln == "package.json": deps.extend(parse_package_json(path, root))
    total_vulns, osv_status = (0, "OFFLINE") if args.offline else query_osv_batch(deps)
    c_files, sast, bins = scan_c_sources_and_bins(root)
    sbom = build_cyclonedx_15(root.name, deps)
    Path(args.sbom_out).write_text(json.dumps(sbom, indent=2), encoding="utf-8")
    print("=========================================================================")
    print(f" CRA-FirmAudit v2.0 Live OSV.dev Scan: {root.name}")
    print("=========================================================================")
    print(f"  * Manifest Dependencies Parsed : {len(deps)}")
    print(f"  * OSV.dev API Status           : {osv_status} ({total_vulns} advisories matched)")
    for d in deps:
        if d.osv_vulns:
            print(f"      [VULN] {d.name}@{d.version or d.commit_sha} -> {[v['id'] for v in d.osv_vulns]}")
    print(f"  * C/C++ Source Files Scanned   : {c_files} ({len(sast)} CWE findings)")
    print(f"  * CycloneDX 1.5 SBOM Written   : {Path(args.sbom_out).resolve()}")
    print("=========================================================================")
    return 0

if __name__ == "__main__":
    sys.exit(main())
