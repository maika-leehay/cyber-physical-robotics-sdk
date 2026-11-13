CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall
BUILD    := build

SOURCES := \
firmware/imu-kalman-filter-mpu6050/main.cpp \
firmware/freertos-socketcan-pid-controller/main.cpp \
ros2_ws/src/amr_description_nav2/src/odometry_benchmark.cpp \
hardware_cad/parametric-sensor-enclosure/src/enclosure_mechanical_verifier.cpp \
control_learning/mujoco-lerobot-act-policy/src/act_benchmark.cpp \
perception/onnx-ros2-vision-node/src/vision_benchmark.cpp \
perception/predictive-maintenance-anomaly-detector/src/anomaly_benchmark.cpp \
perception/gesture-teleop-ros2-streamer/src/teleop_benchmark.cpp \
security/socketcan-ids-iso21434/src/ids_benchmark.cpp \
security/sros2-dds-enclaves/src/sros2_enclave_verifier.cpp \
security/mcuboot-ota-rollback-verifier/src/mcuboot_benchmark.cpp \
security/mqtt-aes-gcm-telemetry-node/src/mqtt_aead_benchmark.cpp \
security/adversarial-vision-patch-defense/src/adv_vision_benchmark.cpp \
security/hardened-ai-policy-safety-interlock/src/interlock_benchmark.cpp

.PHONY: test clean

test:
@mkdir -p $(BUILD)
@for src in $(SOURCES); do \
name=$$(echo $$src | tr '/' '_'); \
$(CXX) $(CXXFLAGS) $$src -o $(BUILD)/$$name && ./$(BUILD)/$$name || exit 1; \
done
python3 security/cra-firmaudit-sbom-scanner/cra_firmaudit_scanner.py

clean:
rm -rf $(BUILD)
