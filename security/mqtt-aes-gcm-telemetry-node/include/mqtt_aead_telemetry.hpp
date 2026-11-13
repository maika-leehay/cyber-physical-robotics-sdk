#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace mechatronics {

struct EncryptedMqttPacket {
    uint64_t iv_nonce;       // 64-bit strictly increasing Initialization Vector (IV)
    uint8_t  ciphertext[16]; // Encrypted sensor telemetry payload
    uint64_t auth_tag;       // 64-bit authenticated integrity tag
};

class SecureTelemetryPipeline {
public:
    explicit SecureTelemetryPipeline(uint64_t key) : master_key_(key), rx_watermark_iv_(0) {}

    uint64_t computeKeystreamBlock(uint64_t iv, uint64_t block_idx) const {
        uint64_t x = master_key_ ^ (iv * 0x9E3779B97F4A7C15ULL) ^ block_idx;
        x ^= (x >> 30); x *= 0xBF58476D1CE4E5B9ULL;
        x ^= (x >> 27); x *= 0x94D049BB133111EBULL;
        return x ^ (x >> 31);
    }

    uint64_t computeAuthTag(uint64_t iv, const uint8_t* ct, size_t len) const {
        uint64_t tag = computeKeystreamBlock(iv, 0xFFFFFFFFULL);
        for (size_t i = 0; i < len; ++i) {
            tag ^= static_cast<uint64_t>(ct[i]) << ((i % 8) * 8);
            tag *= 1099511628211ULL;
        }
        return tag;
    }

    EncryptedMqttPacket encryptTelemetry(uint64_t iv, float imu_roll_deg, float motor_temp_c, uint32_t status) const {
        uint8_t pt[16] = {0};
        std::memcpy(&pt[0], &imu_roll_deg, 4);
        std::memcpy(&pt[4], &motor_temp_c, 4);
        std::memcpy(&pt[8], &status, 4);

        uint64_t ks0 = computeKeystreamBlock(iv, 0);
        uint64_t ks1 = computeKeystreamBlock(iv, 1);
        uint8_t ks[16];
        std::memcpy(&ks[0], &ks0, 8);
        std::memcpy(&ks[8], &ks1, 8);

        EncryptedMqttPacket pkt{};
        pkt.iv_nonce = iv;
        for (size_t i = 0; i < 16; ++i) pkt.ciphertext[i] = pt[i] ^ ks[i];
        pkt.auth_tag = computeAuthTag(iv, pkt.ciphertext, 16);
        return pkt;
    }

    bool decryptAndVerify(const EncryptedMqttPacket& pkt, float& roll_out, float& temp_out) {
        if (pkt.iv_nonce <= rx_watermark_iv_) return false;
        if (computeAuthTag(pkt.iv_nonce, pkt.ciphertext, 16) != pkt.auth_tag) return false;

        uint64_t ks0 = computeKeystreamBlock(pkt.iv_nonce, 0);
        uint64_t ks1 = computeKeystreamBlock(pkt.iv_nonce, 1);
        uint8_t ks[16], pt[16];
        std::memcpy(&ks[0], &ks0, 8);
        std::memcpy(&ks[8], &ks1, 8);
        for (size_t i = 0; i < 16; ++i) pt[i] = pkt.ciphertext[i] ^ ks[i];

        std::memcpy(&roll_out, &pt[0], 4);
        std::memcpy(&temp_out, &pt[4], 4);
        rx_watermark_iv_ = pkt.iv_nonce;
        return true;
    }

private:
    uint64_t master_key_;
    uint64_t rx_watermark_iv_;
};

} // namespace mechatronics
