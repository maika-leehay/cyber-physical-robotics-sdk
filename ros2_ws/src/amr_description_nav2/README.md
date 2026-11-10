# ROS 2 Jazzy Autonomous Mobile Robot (AMR): URDF, RK2 Odometry & LiDAR Avoidance

Differential-drive autonomous mobile robot package implementing a parametric **URDF/Xacro kinematic model**, **2nd-order Runge-Kutta (RK2) wheel odometry**, **REP-105 TF2 quaternion transforms**, and **3-sector 2D LiDAR obstacle avoidance**.

## 1. Subsystem Components

| File | Engineering Role |
| :--- | :--- |
| `urdf/robot.urdf.xacro` | Parametric 30x25x10 cm chassis (2.5 kg), 45 mm traction wheels (28 cm track), passive front caster, and elevated `laser_frame` with analytical diagonal inertia tensors. |
| `include/diff_drive_odometry.hpp` | 100 Hz 2nd-order Runge-Kutta midpoint arc integrator (`odom` -> `base_link`) eliminating 1st-order Euler tangential drift during continuous turns. |
| `include/lidar_obstacle_avoidance.hpp` | Partitions planar `LaserScan` ranges into Left/Front/Right sectors with proportional caution-zone deceleration (`0.90 m`) and monitored standstill (`<= 0.35 m`). |
| `src/odometry_benchmark.cpp` | Deterministic trajectory verification harness validating pose integration and unit quaternion normalization (`||q|| = 1.0000`). |

## 2. Verified Kinematic & Control Benchmarks

| Verification Metric | Measured Result | Target Specification |
| :--- | :---: | :---: |
| **Odometry Integration Method** | **2nd-Order Runge-Kutta (Midpoint Arc)** | Zero tangential chord drift on constant-curvature arcs |
| **TF2 Quaternion Norm (`||q||`)** | **`1.0000`** (`[0, 0, 0.0862, 0.9963]`) | Unit norm compliance per ROS 2 REP-105 |
| **Caution Zone Deceleration Window** | **`0.90 m` -> `0.35 m`** | Smooth linear velocity ramp (`0.50 m/s` down to `0.075 m/s`) |
| **Critical Standstill Threshold** | **`<= 0.35 m`** | Immediate `v_x = 0.0 m/s` stop + in-place clearance pivot |

## 3. Resume / CV Bullet

* **Designed a differential-drive autonomous mobile robot package in ROS 2 (URDF/Xacro with analytical inertia tensors) and implemented 100 Hz 2nd-order Runge-Kutta wheel odometry and 3-sector LiDAR obstacle avoidance in C++17.**
