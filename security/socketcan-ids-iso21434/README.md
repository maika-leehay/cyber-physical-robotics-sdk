# ISO/SAE 21434 SocketCAN Intrusion Detection System (IDS) & AUTOSAR SecOC Filter

Deterministic C++17 CAN 2.0B / SocketCAN security filter implementing **inter-arrival timing anomaly detection**, **monotonic anti-replay freshness counters**, and **16-bit truncated Keyed-MAC authentication (AUTOSAR SecOC pattern)** for robotic and automotive actuators.

## 1. Threat Model & Defense Architecture
Standard CAN 2.0B frames lack sender authentication, allowing compromised bus nodes to inject spoofed torque/position commands (`0x141`). This module enforces a 3-stage pipeline within an 8-byte frame budget (`[4B Angle | 2B Freshness Counter | 2B Keyed-MAC]`):
1. **Timing Anomaly IDS**: Rejects high-frequency bus flooding injections arriving faster than the `7.0 ms` inter-arrival floor (`100 Hz` nominal loop).
2. **Monotonic Freshness Watermark**: Blocks replay attacks of previously captured valid frames (`freshness_cnt <= last_freshness_cnt`).
3. **Truncated SecOC Keyed-MAC**: Verifies a 16-bit cryptographic tag over `[can_id | target_mdeg | freshness_cnt | shared_secret]` before allowing actuator execution.

## 2. Verified Attack Simulation Results

| Test Scenario | Frame Arrival (`ms`) | Counter | MAC Tag | Security Verdict |
| :--- | :---: | :---: | :---: | :---: |
| **1. Legitimate Motion Command** | `10.0 ms` | `1` | Valid | **`ACCEPTED`** |
| **2. Unauthenticated Bus Spoofing** | `20.0 ms` | `2` | `0xDEAD` (Forged) | **`BLOCKED [MAC_MISMATCH]`** |
| **3. Captured Frame Replay Attack** | `30.0 ms` | `1` (Stale) | Valid | **`BLOCKED [STALE_COUNTER]`** |
| **4. High-Rate Bus Flooding DoS** | `41.2 ms` (`dt=1.2ms`) | `3` | Valid | **`BLOCKED [TIMING_ANOMALY]`** |

## 3. Resume / CV Bullet
* **Engineered an ISO/SAE 21434-aligned CAN bus Intrusion Detection System (IDS) and AUTOSAR SecOC message authentication filter in C++17, combining inter-arrival timing analysis, monotonic freshness counters, and 16-bit truncated Keyed-MACs to block spoofing, replay, and bus-flooding attacks on robotic actuators.**
