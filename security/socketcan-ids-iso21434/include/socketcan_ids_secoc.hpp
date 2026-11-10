#pragma once
#include <cmath>
#include <cstdint>
#include <cstring>

namespace mechatronics {

enum class CanSecurityVerdict : uint8_t {
    ACCEPT_VALID_FRAME         = 0,
    DROP_MAC_SPOOFING_DETECTED = 1,
    DROP_REPLAY_ATTACK_STALE   = 2,
    DROP_TIMING_DOS_FLOOD      = 3
};

struct SecuredCanCommandFrame {
    uint32_t can_id;         // 0x141 actuator command ID
    int32_t  target_mdeg;    // Target joint angle in milli-degrees (bytes 0..3)
    uint16_t freshness_cnt;  // Monotonic anti-replay counter (bytes 4..5)
    uint16_t mac_tag;        // 16-bit truncated keyed authentication tag (bytes 6..7)
};

class SocketCanIdsSecOC {
public:
    explicit SocketCanIdsSecOC(uint64_t shared_secret_key, double min_interval_ms = 7.0)
        : secret_key_(shared_secret_key),
          min_interval_ms_(min_interval_ms),
          last_freshness_cnt_(0),
          last_rx_time_ms_(-100.0) {}

    // Computes 16-bit truncated Keyed-MAC over [can_id | target_mdeg | freshness_cnt | secret_key]
    uint16_t computeTruncatedMac(uint32_t can_id, int32_t target_mdeg, uint16_t freshness_cnt) const {
        uint64_t hash = 14695981039346656037ULL ^ secret_key_;
        auto mixByte = [&](uint8_t b) {
            hash ^= static_cast<uint64_t>(b);
            hash *= 1099511628211ULL;
        };
        for (int i = 0; i < 4; ++i) mixByte(static_cast<uint8_t>((can_id >> (i * 8)) & 0xFF));
        for (int i = 0; i < 4; ++i) mixByte(static_cast<uint8_t>((static_cast<uint32_t>(target_mdeg) >> (i * 8)) & 0xFF));
        mixByte(static_cast<uint8_t>(freshness_cnt & 0xFF));
        mixByte(static_cast<uint8_t>((freshness_cnt >> 8) & 0xFF));

        return static_cast<uint16_t>((hash ^ (hash >> 32)) & 0xFFFF);
    }

    // Signs an outgoing actuator command frame
    SecuredCanCommandFrame signCommand(uint32_t can_id, float target_deg, uint16_t freshness_cnt) const {
        int32_t mdeg = static_cast<int32_t>(target_deg * 1000.0f);
        uint16_t mac = computeTruncatedMac(can_id, mdeg, freshness_cnt);
        return SecuredCanCommandFrame{ can_id, mdeg, freshness_cnt, mac };
    }

    // Audits incoming CAN frame against Timing IDS, Anti-Replay, and Cryptographic MAC
    CanSecurityVerdict inspectFrame(const SecuredCanCommandFrame& frame, double rx_time_ms) {
        // 1. Inter-Arrival Timing Anomaly Check (Blocks bus injection / flooding < 7.0 ms)
        if ((rx_time_ms - last_rx_time_ms_) < min_interval_ms_) {
            return CanSecurityVerdict::DROP_TIMING_DOS_FLOOD;
        }

        // 2. Monotonic Freshness Counter Check (Blocks captured frame replay attacks)
        if (frame.freshness_cnt <= last_freshness_cnt_) {
            return CanSecurityVerdict::DROP_REPLAY_ATTACK_STALE;
        }

        // 3. Cryptographic SecOC MAC Verification (Blocks unauthenticated bus spoofing)
        uint16_t expected_mac = computeTruncatedMac(frame.can_id, frame.target_mdeg, frame.freshness_cnt);
        if (frame.mac_tag != expected_mac) {
            return CanSecurityVerdict::DROP_MAC_SPOOFING_DETECTED;
        }

        // Frame authenticated: advance anti-replay watermark and timestamp
        last_freshness_cnt_ = frame.freshness_cnt;
        last_rx_time_ms_    = rx_time_ms;
        return CanSecurityVerdict::ACCEPT_VALID_FRAME;
    }

private:
    uint64_t secret_key_;
    double   min_interval_ms_;
    uint16_t last_freshness_cnt_;
    double   last_rx_time_ms_;
};

} // namespace mechatronics
