# Cyber-Physical Robotics, Edge AI & Embedded Security SDK

Production-grade **C++17 embedded firmware**, **ROS 2 Jazzy autonomy packages**, **Edge AI perception pipelines**, and **Cyber-Physical security modules** validated via deterministic Hardware-in-the-Loop (HIL) benchmarks.

## Subsystem Architecture & Verified Modules

| Subsystem Module | Core Stack & Protocols | Key Engineering Benchmark | Status |
| :--- | :--- | :--- | :---: |
| **[`firmware/imu-kalman-filter-mpu6050`](./firmware/imu-kalman-filter-mpu6050)** | C++17, STM32/ESP32 I2C HAL (`0x68`), 2-State Kalman Filter, CRC-8 UART | **92.3% orientation RMSE reduction** (`10.53°` to `0.81°`) @ 200 Hz with thermal gyro bias tracking | Verified |
| **[`firmware/freertos-socketcan-pid-controller`](./firmware/freertos-socketcan-pid-controller)** | C++17, FreeRTOS (1 kHz / 100 Hz tasks), SocketCAN (CAN 2.0B/FD), 4x Encoder LUT | **`< 0.01°` steady-state error**, `1.42%` overshoot, and `82 ms` torque disturbance recovery | Verified |
| **[`ros2_ws/src/amr_description_nav2`](./ros2_ws/src/amr_description_nav2)** | ROS 2 Jazzy, Gazebo, URDF/Xacro, RK2 Odometry, TF2, 2D LiDAR Avoidance | Analytical inertia tensors, zero-drift midpoint arc odometry & 3-sector safety stop | Verified |
| **[`perception/onnx-ros2-vision-node`](./perception/onnx-ros2-vision-node)** | C++17, ONNX Runtime Post-Processing, IoU NMS, RGB-D Pinhole Deprojection | Real-time 2D-to-3D metric grasp pose back-projection (`camera_optical_frame`) | Verified |
| **`security/sros2-dds-enclaves`** | SROS2, DDS-Security, X.509 Certs, AES-256-GCM Governance | Least-privilege XML topic access control blocking rogue `/cmd_vel` injection | Planned |
| **`security/cra-firmaudit-sbom-scanner`** | Python, Binwalk, CycloneDX 1.5 SBOM, MCUboot ECDSA Verification | Automated EU Cyber Resilience Act (CRA) binary audit & CVE reporting | Planned |
