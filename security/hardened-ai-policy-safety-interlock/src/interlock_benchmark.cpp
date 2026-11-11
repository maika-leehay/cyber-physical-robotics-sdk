#include "../include/hardened_policy_interlock.hpp"
#include <iostream>

int main() {
    using namespace mechatronics;
    HardenedPolicySafetyInterlock interlock(2.50, 15.0, 0.08);

    RobotActionPacket pkt_valid   = { {'S','A','F','E'}, 1.20,  8.5, 0.500, 0.505 };
    RobotActionPacket pkt_pickle  = { {0x80, 0x04, 0x95, 0x00}, 1.00, 5.0, 0.500, 0.500 };
    RobotActionPacket pkt_fdi     = { {'S','A','F','E'}, 1.10,  7.0, 1.450, 0.510 };
    RobotActionPacket pkt_runaway = { {'S','A','F','E'}, 6.80, 24.0, 0.520, 0.518 };

    auto v1 = interlock.auditActionPacket(pkt_valid);
    auto v2 = interlock.auditActionPacket(pkt_pickle);
    auto v3 = interlock.auditActionPacket(pkt_fdi);
    auto v4 = interlock.auditActionPacket(pkt_runaway);

    std::cout << "=========================================================================\n";
    std::cout << " Phase 4 Capstone: AI Policy Server RCE Guard & ISO 10218-1:2025 Interlock\n";
    std::cout << "=========================================================================\n"
              << "  * Test 1 (Valid Safetensors Policy Action)  : " << (v1 == PolicySafetyVerdict::ALLOW_SAFE_TRAJECTORY ? "ALLOWED" : "FAIL") << "\n"
              << "  * Test 2 (CVE-2026-25874 Pickle RCE Payload): " << (v2 == PolicySafetyVerdict::BLOCK_CVE_2026_25874_PICKLE ? "BLOCKED [UNSAFE_PICKLE_OPCODE]" : "FAIL") << "\n"
              << "  * Test 3 (False Data Injection Sensor Spoof): " << (v3 == PolicySafetyVerdict::BLOCK_FDI_TELEMETRY_ANOMALY ? "BLOCKED [FDI_RESIDUAL_EXCEEDED]" : "FAIL") << "\n"
              << "  * Test 4 (Adversarial Policy Torque Spike)  : " << (v4 == PolicySafetyVerdict::TRIGGER_ISO10218_STANDSTILL ? "HALTED  [ISO10218_STANDSTILL]" : "FAIL") << "\n";

    return (v1 == PolicySafetyVerdict::ALLOW_SAFE_TRAJECTORY &&
            v2 == PolicySafetyVerdict::BLOCK_CVE_2026_25874_PICKLE &&
            v3 == PolicySafetyVerdict::BLOCK_FDI_TELEMETRY_ANOMALY &&
            v4 == PolicySafetyVerdict::TRIGGER_ISO10218_STANDSTILL) ? 0 : 1;
}
