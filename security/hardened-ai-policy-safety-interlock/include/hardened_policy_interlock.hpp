#pragma once
#include <cmath>
#include <cstdint>
#include <cstring>

namespace mechatronics {

enum class PolicySafetyVerdict : uint8_t {
    ALLOW_SAFE_TRAJECTORY         = 0,
    BLOCK_CVE_2026_25874_PICKLE   = 1,
    BLOCK_FDI_TELEMETRY_ANOMALY   = 2,
    TRIGGER_ISO10218_STANDSTILL   = 3
};

struct RobotActionPacket {
    uint8_t magic_header[4];   // Must be "SAFE" (0x53,0x41,0x46,0x45); rejects Python Pickle (0x80)
    double  cmd_joint_vel_rps; // Commanded joint velocity from VLA/ACT policy (rad/s)
    double  cmd_joint_trq_nm;  // Commanded joint torque (N*m)
    double  reported_pos_rad;  // Sensor feedback position (rad)
    double  kalman_pred_rad;   // Internal state-estimator predicted position (rad)
};

class HardenedPolicySafetyInterlock {
public:
    HardenedPolicySafetyInterlock(double max_vel_rps = 2.50, double max_torque_nm = 15.0, double fdi_residual_rad = 0.08)
        : max_vel_(max_vel_rps), max_trq_(max_torque_nm), max_fdi_res_(fdi_residual_rad) {}

    PolicySafetyVerdict auditActionPacket(const RobotActionPacket& pkt) const {
        // 1. Mitigate CVE-2026-25874: Reject Python Pickle opcodes (PROTO 0x80) at transport boundary
        if (pkt.magic_header[0] == 0x80 || std::memcmp(pkt.magic_header, "SAFE", 4) != 0) {
            return PolicySafetyVerdict::BLOCK_CVE_2026_25874_PICKLE;
        }

        // 2. False Data Injection (FDI) State-Estimator Residual Check
        double innovation_residual = std::abs(pkt.reported_pos_rad - pkt.kalman_pred_rad);
        if (innovation_residual > max_fdi_res_) {
            return PolicySafetyVerdict::BLOCK_FDI_TELEMETRY_ANOMALY;
        }

        // 3. ISO 10218-1:2025 Power, Force & Velocity Limiting -> Safe Monitored Standstill
        if (std::abs(pkt.cmd_joint_vel_rps) > max_vel_ || std::abs(pkt.cmd_joint_trq_nm) > max_trq_) {
            return PolicySafetyVerdict::TRIGGER_ISO10218_STANDSTILL;
        }

        return PolicySafetyVerdict::ALLOW_SAFE_TRAJECTORY;
    }

private:
    double max_vel_;
    double max_trq_;
    double max_fdi_res_;
};

} // namespace mechatronics
