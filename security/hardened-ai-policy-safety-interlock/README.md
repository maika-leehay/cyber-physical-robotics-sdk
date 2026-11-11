# Hardened AI Robot Policy Interlock: CVE-2026-25874 Mitigation, FDI Filter & ISO 10218-1:2025 Watchdog

C++17 cyber-physical safety supervisor uniting **AI policy transport hardening**, **Kalman residual False Data Injection (FDI) detection**, and **ISO 10218-1:2025 monitored standstill enforcement** for collaborative robotic manipulators.

## 1. Threat Model & Three-Layer Defense
1. **CVE-2026-25874 Pickle RCE Guard**: Rejects untrusted Python `pickle` serialization opcodes (`0x80 0x04/0x05`) at the gRPC/socket boundary, enforcing strictly typed `SAFE` (`safetensors` / Protobuf) tensor framing.
2. **Kalman Innovation Residual FDI Detector**: Flags man-in-the-middle sensor telemetry spoofing whenever reported encoder feedback diverges from the internal state-estimator prediction beyond `0.08 rad`.
3. **ISO 10218-1:2025 Power & Force Interlock**: Overrides adversarial or hallucinated neural policy outputs that exceed physical joint velocity (`2.50 rad/s`) or torque (`15.0 N*m`) envelopes, asserting a Category 2 Safe Monitored Standstill.

## 2. Verified Capstone Benchmark Matrix

| Test Vector | Wire Header | Residual (`rad`) | Torque (`N*m`) | Supervisor Verdict |
| :--- | :---: | :---: | :---: | :---: |
| **1. Nominal VLA/ACT Policy Step** | `SAFE` | `0.005 rad` | `8.5 N*m` | **`ALLOWED`** |
| **2. CVE-2026-25874 Exploit Payload** | `0x80 0x04` (Pickle) | `0.000 rad` | `5.0 N*m` | **`BLOCKED [UNSAFE_PICKLE_OPCODE]`** |
| **3. Sensor False Data Injection (FDI)** | `SAFE` | **`0.940 rad`** | `7.0 N*m` | **`BLOCKED [FDI_RESIDUAL_EXCEEDED]`** |
| **4. Adversarial Policy Torque Spike** | `SAFE` | `0.002 rad` | **`24.0 N*m`** | **`HALTED [ISO10218_STANDSTILL]`** |

## 3. Resume / CV Bullet
* **Engineered a C++17 cyber-physical safety supervisor for AI-driven robotic manipulators, mitigating CVE-2026-25874-class pickle deserialization RCEs, blocking False Data Injection (FDI) via Kalman innovation residuals, and enforcing ISO 10218-1:2025 monitored standstill limits.**
