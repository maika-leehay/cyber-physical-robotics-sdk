# SROS2 DDS-Security Architecture: X.509 Enclaves & Least-Privilege Topic Governance

ROS 2 Jazzy middleware hardening module enforcing **OMG DDS-Security (SROS2)** mutual X.509 authentication, **AES-256-GCM RTPS encryption**, and deny-by-default XML topic access control to prevent unauthorized `/cmd_vel` velocity injection.

## 1. Security Architecture & Threat Mitigation
Unsecured ROS 2 DDS discovery allows any participant on the local subnet to publish malicious actuator commands on `rt/cmd_vel`. This module enforces two defence-in-depth barriers:
1. **PKI Identity Verification (`ca.cert.pem`)**: Rejects DDS participants lacking an X.509 certificate signed by the robot's offline Root Certificate Authority (`DENY_UNAUTHENTICATED_CERT`).
2. **Fine-Grained Enclave Permissions (`policies/permissions.xml`)**: Isolates `/amr/lidar_driver` and `/amr/nav2_controller` into separate cryptographic enclaves with `<default>DENY</default>` topic rules, ensuring a compromised sensor driver cannot pivot to publish motion commands (`DENY_TOPIC_WRITE_VIOLATION`).

## 2. Verified Access-Control Matrix

| DDS Enclave Identity | X.509 CA Status | Target Topic | Policy Verdict |
| :--- | :---: | :---: | :---: |
| `/amr/nav2_controller` | Verified | `rt/cmd_vel` | **`ALLOWED [AES-256-GCM]`** |
| `/amr/lidar_driver` | Verified | `rt/cmd_vel` | **`BLOCKED [LEAST_PRIVILEGE_XML]`** |
| `/rogue_lan_node` | Unsigned / Forged | `rt/cmd_vel` | **`BLOCKED [INVALID_X509_CA]`** |

## 3. Resume / CV Bullet
* **Hardened a ROS 2 Jazzy autonomous navigation stack using SROS2 and OMG DDS-Security (X.509 PKI enclaves and AES-256-GCM RTPS encryption), authoring least-privilege XML topic governance policies to block unauthorized `/cmd_vel` command injection.**
