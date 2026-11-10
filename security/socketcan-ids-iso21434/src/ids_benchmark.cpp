#include "../include/socketcan_ids_secoc.hpp"
#include <iostream>

int main() {
    using namespace mechatronics;
    SocketCanIdsSecOC secoc_ids(/*shared_secret_key=*/ 0xA5F09C3E71B284D1ULL, /*min_interval_ms=*/ 7.0);

    // 1. Legitimate signed command at t = 10.0 ms (Counter = 1, Angle = 45.0 deg)
    auto valid_frame = secoc_ids.signCommand(0x141, 45.0f, 1);
    auto v1 = secoc_ids.inspectFrame(valid_frame, 10.0);

    // 2. Attacker injects unauthenticated spoofed frame at t = 20.0 ms (forged MAC 0xDEAD)
    SecuredCanCommandFrame spoof_frame{ 0x141, 180000, 2, 0xDEAD };
    auto v2 = secoc_ids.inspectFrame(spoof_frame, 20.0);

    // 3. Attacker replays captured valid_frame (Counter = 1) at t = 30.0 ms
    auto v3 = secoc_ids.inspectFrame(valid_frame, 30.0);

    // 4. Legitimate frame 2 at t = 40.0 ms followed by Bus Flood injection 1.2 ms later (t = 41.2 ms)
    auto valid_frame2 = secoc_ids.signCommand(0x141, 60.0f, 2);
    auto v4 = secoc_ids.inspectFrame(valid_frame2, 40.0);
    auto flood_frame  = secoc_ids.signCommand(0x141, 90.0f, 3);
    auto v5 = secoc_ids.inspectFrame(flood_frame, 41.2);

    std::cout << "=========================================================================\n";
    std::cout << " ISO/SAE 21434 SocketCAN Intrusion Detection & SecOC MAC Verification\n";
    std::cout << "=========================================================================\n"
              << "  * Test 1 (Legitimate Signed Frame @ 10.0ms) : " << (v1 == CanSecurityVerdict::ACCEPT_VALID_FRAME ? "ACCEPTED" : "FAIL") << "\n"
              << "  * Test 2 (Forged MAC Spoofing Attack)       : " << (v2 == CanSecurityVerdict::DROP_MAC_SPOOFING_DETECTED ? "BLOCKED [MAC_MISMATCH]" : "FAIL") << "\n"
              << "  * Test 3 (Captured Frame Replay Attack)     : " << (v3 == CanSecurityVerdict::DROP_REPLAY_ATTACK_STALE ? "BLOCKED [STALE_COUNTER]" : "FAIL") << "\n"
              << "  * Test 4 (Legitimate Signed Frame @ 40.0ms) : " << (v4 == CanSecurityVerdict::ACCEPT_VALID_FRAME ? "ACCEPTED" : "FAIL") << "\n"
              << "  * Test 5 (1.2ms Bus Flooding Injection)     : " << (v5 == CanSecurityVerdict::DROP_TIMING_DOS_FLOOD ? "BLOCKED [TIMING_ANOMALY]" : "FAIL") << "\n";

    return (v1 == CanSecurityVerdict::ACCEPT_VALID_FRAME &&
            v2 == CanSecurityVerdict::DROP_MAC_SPOOFING_DETECTED &&
            v3 == CanSecurityVerdict::DROP_REPLAY_ATTACK_STALE &&
            v4 == CanSecurityVerdict::ACCEPT_VALID_FRAME &&
            v5 == CanSecurityVerdict::DROP_TIMING_DOS_FLOOD) ? 0 : 1;
}
