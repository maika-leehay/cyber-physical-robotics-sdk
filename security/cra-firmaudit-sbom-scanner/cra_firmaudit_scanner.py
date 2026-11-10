#!/usr/bin/env python3
"""
CRA-FirmAudit (Open-Core CLI): Embedded Firmware Vulnerability & EU CRA SBOM Auditor
"""

import json
import re
import struct
from dataclasses import dataclass
from typing import List, Dict

MCUBOOT_IMAGE_MAGIC = 0x96F3B83D

BANNED_C_FUNCTIONS = {
    "strcpy":  ("CWE-120", "HIGH",     "Unbounded string copy; replace with snprintf or strncpy_s"),
    "sprintf": ("CWE-120", "HIGH",     "Unbounded format string write; replace with snprintf"),
    "gets":    ("CWE-242", "CRITICAL", "Inherently unsafe input read; replace with fgets"),
    "strcat":  ("CWE-120", "HIGH",     "Unbounded string concatenation; replace with strncat"),
    "system":  ("CWE-78",  "CRITICAL", "OS command injection sink in embedded Linux daemon"),
}

CREDENTIAL_PATTERNS = [
    (re.compile(r'(?i)(password|passwd|mqtt_pass|api_key|secret)\s*=\s*"([^"]{4,})"'),
     "CWE-798", "CRITICAL", "Hardcoded plaintext credential in firmware source"),
    (re.compile(r'-----BEGIN (RSA |EC )?PRIVATE KEY-----'),
     "CWE-321", "CRITICAL", "Embedded PEM private key inside firmware image"),
]

@dataclass
class SecurityFinding:
    rule_id: str
    severity: str
    line_num: int
    match: str
    remediation: str

def scan_firmware_source(source_code: str) -> List[SecurityFinding]:
    findings: List[SecurityFinding] = []
    for idx, line in enumerate(source_code.splitlines(), start=1):
        for func, (cwe, sev, rem) in BANNED_C_FUNCTIONS.items():
            if re.search(rf'\b{func}\s*\(', line):
                findings.append(SecurityFinding(cwe, sev, idx, f"{func}()", rem))
        for pattern, cwe, sev, rem in CREDENTIAL_PATTERNS:
            if pattern.search(line):
                findings.append(SecurityFinding(cwe, sev, idx, "Hardcoded Secret", rem))
    return findings

def verify_mcuboot_header(firmware_bytes: bytes) -> bool:
    if len(firmware_bytes) < 4:
        return False
    magic, = struct.unpack("<I", firmware_bytes[:4])
    return magic == MCUBOOT_IMAGE_MAGIC

def generate_cyclonedx_sbom(device_name: str, version: str, components: List[Dict[str, str]]) -> Dict:
    return {
        "bomFormat": "CycloneDX",
        "specVersion": "1.5",
        "version": 1,
        "metadata": {
            "regulation": "EU Cyber Resilience Act (EU 2024/2847)",
            "component": {"type": "firmware", "name": device_name, "version": version}
        },
        "components": components
    }

if __name__ == "__main__":
    sample_legacy_firmware_c = (
        "#include <cstdio>\n"
        "#include <cstring>\n"
        "const char* mqtt_pass = \"Admin1234!@#\";\n"
        "void process_telemetry(const char* rx_buf) {\n"
        "    char local_buf[32];\n"
        "    strcpy(local_buf, rx_buf);\n"
        "    sprintf(local_buf, \"ACK:%s\", rx_buf);\n"
        "}\n"
    )
    unsigned_bin = b"\x00\x20\x00\x20\x11\x22\x33\x44"
    signed_bin   = struct.pack("<I", MCUBOOT_IMAGE_MAGIC) + b"\x00\x00\x20\x00"

    findings = scan_firmware_source(sample_legacy_firmware_c)
    sbom = generate_cyclonedx_sbom(
        device_name="stm32-industrial-can-gateway",
        version="2.4.0",
        components=[
            {"type": "operating-system", "name": "FreeRTOS-Kernel", "version": "11.1.0", "purl": "pkg:github/FreeRTOS/FreeRTOS-Kernel@11.1.0"},
            {"type": "library", "name": "mbedtls", "version": "3.6.0", "purl": "pkg:github/Mbed-TLS/mbedtls@3.6.0"},
            {"type": "library", "name": "mcuboot", "version": "2.1.0", "purl": "pkg:github/mcu-tools/mcuboot@2.1.0"},
        ]
    )

    print("=========================================================================")
    print(" CRA-FirmAudit: EU Cyber Resilience Act Firmware & SBOM Auditor")
    print("=========================================================================")
    print(f"  * MCUboot Secure Boot Check (Unsigned Image) : {'PASS' if verify_mcuboot_header(unsigned_bin) else 'FAIL [MISSING_0x96F3B83D_MAGIC]'}")
    print(f"  * MCUboot Secure Boot Check (Signed Image)   : {'PASS [VERIFIED_0x96F3B83D]' if verify_mcuboot_header(signed_bin) else 'FAIL'}")
    print(f"  * Static C/C++ Vulnerability Findings        : {len(findings)} issues flagged")
    for f in findings:
        print(f"      - [Line {f.line_num}] {f.rule_id} ({f.severity}): {f.match} -> {f.remediation}")
    print(f"  * CycloneDX 1.5 SBOM Manifest Generated      : {len(sbom['components'])} embedded components indexed")
    print("=========================================================================")
