#pragma once
#include <cstdint>
#include <cstring>

namespace mechatronics {

// ============================================================================
// Standard 8-Byte Industrial CAN 2.0B / SocketCAN Frame Structure
// ============================================================================
struct CanFrame {
    uint32_t can_id;   // 11-bit standard or 29-bit extended arbitration ID
    uint8_t  can_dlc;  // Data Length Code (0 to 8 bytes)
    uint8_t  data[8];  // Payload data
};

namespace can_protocol {
constexpr uint32_t ID_CMD_TARGET_ANGLE  = 0x141; // High-priority position command
constexpr uint32_t ID_FEEDBACK_ACTUATOR = 0x241; // Periodic status/telemetry feedback

// Encodes target angle (-180.0 to +180.0 deg) into 8-byte CAN command frame
inline CanFrame packTargetCommand(float target_deg, float max_velocity_dps) {
    CanFrame frame{};
    frame.can_id = ID_CMD_TARGET_ANGLE;
    frame.can_dlc = 8;

    // Scale target angle: int32 (0.01 deg per count)
    int32_t angle_counts = static_cast<int32_t>(target_deg * 100.0f);
    uint32_t vel_counts   = static_cast<uint32_t>(max_velocity_dps * 100.0f);

    std::memcpy(&frame.data[0], &angle_counts, sizeof(angle_counts));
    std::memcpy(&frame.data[4], &vel_counts, sizeof(vel_counts));
    return frame;
}

// Unpacks target angle command frame
inline void unpackTargetCommand(const CanFrame& frame, float& target_deg, float& max_vel_dps) {
    int32_t angle_counts = 0;
    uint32_t vel_counts   = 0;
    std::memcpy(&angle_counts, &frame.data[0], sizeof(angle_counts));
    std::memcpy(&vel_counts,   &frame.data[4], sizeof(vel_counts));
    target_deg  = static_cast<float>(angle_counts) / 100.0f;
    max_vel_dps = static_cast<float>(vel_counts)   / 100.0f;
}

// Encodes actuator telemetry feedback into 8-byte CAN frame
inline CanFrame packActuatorFeedback(float measured_deg, float motor_pwm, uint8_t status_flags) {
    CanFrame frame{};
    frame.can_id = ID_FEEDBACK_ACTUATOR;
    frame.can_dlc = 8;

    int32_t angle_counts = static_cast<int32_t>(measured_deg * 100.0f);
    int16_t pwm_counts   = static_cast<int16_t>(motor_pwm * 100.0f);

    std::memcpy(&frame.data[0], &angle_counts, sizeof(angle_counts));
    std::memcpy(&frame.data[4], &pwm_counts, sizeof(pwm_counts));
    frame.data[6] = status_flags; // Bit 0: Closed-Loop Active, Bit 1: Target Reached
    frame.data[7] = 0x00;         // Reserved / Checksum
    return frame;
}

} // namespace can_protocol

} // namespace mechatronics
