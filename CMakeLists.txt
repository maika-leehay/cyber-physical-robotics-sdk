cmake_minimum_required(VERSION 3.16)
project(CyberPhysicalRoboticsSDK VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-O2 -Wall)

enable_testing()

set(HIL_TARGETS
    imu_kalman:firmware/imu-kalman-filter-mpu6050/main.cpp
    freertos_pid:firmware/freertos-socketcan-pid-controller/main.cpp
    ros2_odom:ros2_ws/src/amr_description_nav2/src/odometry_benchmark.cpp
    cad_verifier:hardware_cad/parametric-sensor-enclosure/src/enclosure_mechanical_verifier.cpp
    sim2real_act:control_learning/mujoco-lerobot-act-policy/src/act_benchmark.cpp
    onnx_vision:perception/onnx-ros2-vision-node/src/vision_benchmark.cpp
    pdm_anomaly:perception/predictive-maintenance-anomaly-detector/src/anomaly_benchmark.cpp
    gesture_teleop:perception/gesture-teleop-ros2-streamer/src/teleop_benchmark.cpp
    socketcan_ids:security/socketcan-ids-iso21434/src/ids_benchmark.cpp
    sros2_enclaves:security/sros2-dds-enclaves/src/sros2_enclave_verifier.cpp
    mcuboot_ota:security/mcuboot-ota-rollback-verifier/src/mcuboot_benchmark.cpp
    mqtt_aead:security/mqtt-aes-gcm-telemetry-node/src/mqtt_aead_benchmark.cpp
    adv_vision:security/adversarial-vision-patch-defense/src/adv_vision_benchmark.cpp
    ai_interlock:security/hardened-ai-policy-safety-interlock/src/interlock_benchmark.cpp
)

foreach(entry IN LISTS HIL_TARGETS)
    string(REPLACE ":" ";" pair "${entry}")
    list(GET pair 0 target_name)
    list(GET pair 1 src_path)
    add_executable(${target_name} ${src_path})
    add_test(NAME HIL_${target_name} COMMAND ${target_name})
endforeach()
