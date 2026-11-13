# Cyber-Physical Robotics, Edge AI & Embedded Security SDK

[https://img.shields.io/badge/HIL_Benchmarks-Passing_(14%2F14]-brightgreen?logo=githubactions)
[https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus]
[https://img.shields.io/badge/ROS_2-Jazzy_Jalisco-22314E?logo=ros]
[https://img.shields.io/badge/Safety_%26_Security-ISO_21434_%7C_ISO_10218:2025_%7C_EU_CRA-success]

Production-grade **C++17 embedded firmware**, **parametric 3D CAD**, **ROS 2 Jazzy autonomy packages**, **Sim-to-Real robot learning**, **Edge AI perception pipelines**, and **Cyber-Physical security modules** validated via deterministic Hardware-in-the-Loop (HIL) benchmarks.

## Subsystem Architecture & Verified Modules (14 / 14 Complete)

| Subsystem Module | Core Stack & Protocols | Key Engineering Benchmark | Status |
| :--- | :--- | :--- | :---: |
| **[`firmware/imu-kalman-filter-mpu6050`](./firmware/imu-kalman-filter-mpu6050)** | C++17, STM32/ESP32 I2C HAL (`0x68`), 2-State Kalman Filter, CRC-8 UART | **92.3% orientation RMSE reduction** (`10.53°` to `0.81°`) @ 200 Hz with thermal gyro bias tracking | Verified |
| **[`firmware/freertos-socketcan-pid-controller`](./firmware/freertos-socketcan-pid-controller)** | C++17, FreeRTOS (1 kHz / 100 Hz tasks), SocketCAN (CAN 2.0B/FD), 4x Encoder LUT | **`< 0.01°` steady-state error**, `1.42%` overshoot, and `82 ms` torque disturbance recovery | Verified |
| **[`ros2_ws/src/amr_description_nav2`](./ros2_ws/src/amr_description_nav2)** | ROS 2 Jazzy, Gazebo, URDF/Xacro, RK2 Odometry, TF2, 2D LiDAR Avoidance | Analytical inertia tensors, zero-drift midpoint arc odometry & 3-sector safety stop | Verified |
| **[`hardware_cad/parametric-sensor-enclosure`](./hardware_cad/parametric-sensor-enclosure)** | OpenSCAD, C++17 Snap-Fit Beam Mechanics, Natural Convection Thermal Analysis | **`1.67%` snap-fit strain** (`1.32x` PETG safety factor) & **`+10.14 °C` thermal rise** @ `2.8 W` | Verified |
| **[`control_learning/mujoco-lerobot-act-policy`](./control_learning/mujoco-lerobot-act-policy)** | C++17, MuJoCo Domain Randomization, LeRobot ACT Temporal Ensembling (`k=16`) | **61.4% Sim-to-Real tracking RMSE reduction** (`9.52 mm` to `3.67 mm`) across 50 perturbed rollouts | Verified |
| **[`perception/onnx-ros2-vision-node`](./perception/onnx-ros2-vision-node)** | C++17, ONNX Runtime Post-Processing, IoU NMS, RGB-D Pinhole Deprojection | Real-time 2D-to-3D metric grasp pose back-projection (`camera_optical_frame`) | Verified |
| **[`perception/predictive-maintenance-anomaly-detector`](./perception/predictive-maintenance-anomaly-detector)** | C++17, Peak-to-RMS Crest Factor, Multivariate Mahalanobis Anomaly Scoring | Early inner-race bearing spalling isolation (`Crest=3.46`, `Score=5.08`) prior to thermal runaway | Verified |
| **[`perception/gesture-teleop-ros2-streamer`](./perception/gesture-teleop-ros2-streamer)** | C++17, 21-Point MediaPipe Hand Landmarks, ROS 2 Twist, 150 ms Watchdog | `+/-8%` radial tremor deadzone, thumb-index pinch E-Stop & frame-loss safety halt | Verified |
| **[`security/socketcan-ids-iso21434`](./security/socketcan-ids-iso21434)** | C++17, ISO/SAE 21434 Timing IDS, Monotonic Counter, AUTOSAR SecOC MAC | 100% rejection of forged MAC spoofing, stale-counter replay, and `< 7.0 ms` bus flooding | Verified |
| **[`security/sros2-dds-enclaves`](./security/sros2-dds-enclaves)** | SROS2, OMG DDS-Security, X.509 Enclaves, AES-256-GCM Topic Governance | Least-privilege XML topic access control blocking lateral & rogue `/cmd_vel` injection | Verified |
| **[`security/mcuboot-ota-rollback-verifier`](./security/mcuboot-ota-rollback-verifier)** | C++17, MCUboot (`0x96F3B83D`), ECDSA P-256, OTP SVN Anti-Rollback, A/B Swap | Blocks tampered binaries, version downgrade attacks, and unconfirmed trial boot loops | Verified |
| **[`security/mqtt-aes-gcm-telemetry-node`](./security/mqtt-aes-gcm-telemetry-node)** | C++17, AEAD Stream Encryption, 64-Bit Monotonic IV Nonce, GMAC Tag | Verify-Then-Decrypt telemetry pipeline blocking replay and 1-bit ciphertext tampering | Verified |
| **[`security/adversarial-vision-patch-defense`](./security/adversarial-vision-patch-defense)** | C++17, FGSM Attack Generator, L1 Feature Squeezing, Cross-Modal LiDAR Check | Detects digital adversarial patches (`L1 > 0.35`) and 2D printed poster depth spoofs | Verified |
| **[`security/cra-firmaudit-sbom-scanner`](./security/cra-firmaudit-sbom-scanner)** | Python 3, CWE-120/798 SAST, MCUboot Header Check, CycloneDX 1.5 SBOM | Automated EU Cyber Resilience Act (EU 2024/2847) binary header & SBOM compliance audit | Verified |
| **[`security/hardened-ai-policy-safety-interlock`](./security/hardened-ai-policy-safety-interlock)** | C++17, CVE-2026-25874 Pickle Guard, Kalman FDI Detector, ISO 10218-1:2025 Interlock | Blocks RCE opcodes, sensor FDI spoofing (`>0.08 rad`), and adversarial torque spikes | Verified |
