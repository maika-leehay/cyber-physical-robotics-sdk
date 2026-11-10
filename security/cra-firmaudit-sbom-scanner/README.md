# CRA-FirmAudit: EU Cyber Resilience Act (EU 2024/2847) Firmware & SBOM Auditor

Automated embedded firmware security auditor and **CycloneDX 1.5 Software Bill of Materials (SBOM)** generator designed for STM32, ESP32, and embedded Linux IoT devices subject to the **EU Cyber Resilience Act (CRA)**.

## 1. Regulatory Problem & Technical Architecture
Under the **EU Cyber Resilience Act (EU 2024/2847)**, connected hardware manufacturers must enforce cryptographic Secure Boot, eliminate exploitable memory-corruption sinks, and maintain a machine-readable **CycloneDX / SPDX SBOM** for 24-hour vulnerability reporting.

This open-core CLI module performs three automated verification checks:
1. **MCUboot Secure Boot Header Verification (`verify_mcuboot_header`)**: Inspects firmware binary images for the `0x96F3B83D` (`MCUBOOT_IMAGE_MAGIC`) cryptographic image header.
2. **Embedded C/C++ SAST & Credential Scanner (`scan_firmware_source`)**: Detects unbounded memory-unsafe C sinks (`CWE-120`: `strcpy`, `sprintf`, `strcat`; `CWE-242`: `gets`; `CWE-78`: `system`) and hardcoded credentials/keys (`CWE-798`, `CWE-321`).
3. **CycloneDX 1.5 SBOM Manifest Export (`generate_cyclonedx_sbom`)**: Produces standardized `pkg:github/...` Package URL (`purl`) component inventories for continuous CVE matching.

## 2. Verified Audit Benchmarks

| Audit Check | Target Input | Verdict / Finding |
| :--- | :--- | :--- |
| **MCUboot Magic Header (`0x96F3B83D`)** | Unsigned Raw Binary | `FAIL [MISSING_0x96F3B83D_MAGIC]` |
| **MCUboot Magic Header (`0x96F3B83D`)** | Signed ECDSA Image | `PASS [VERIFIED_0x96F3B83D]` |
| **Hardcoded Credential Detection** | `mqtt_pass = "..."` | `CRITICAL [CWE-798 Flagged]` |
| **Memory-Unsafe C Sinks** | `strcpy()`, `sprintf()` | `HIGH [CWE-120 Remediation Issued]` |
| **CycloneDX 1.5 SBOM Export** | `FreeRTOS`, `mbedTLS`, `MCUboot` | `PASS [3 PURL Components Indexed]` |

## 3. Commercial B2B Firmware Audits (`CRA-FirmAudit Pro`)
*This directory hosts the open-source CLI verification core. For full binary filesystem extraction (SquashFS/JFFS2/ELF symbol recovery), automated CVE-to-CVSS v4.0 matching, and executive PDF compliance dossier generation for EU CE technical files, reach out via GitHub/LinkedIn for commercial audit engagements.*

## 4. Resume / CV Bullet
* **Developed an automated embedded firmware security auditor and CycloneDX 1.5 SBOM generator in Python aligned with the EU Cyber Resilience Act (EU 2024/2847), detecting CWE-120 memory corruption sinks, CWE-798 hardcoded secrets, and unsigned MCUboot binary headers.**
