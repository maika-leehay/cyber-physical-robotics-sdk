#include "../include/mqtt_aead_telemetry.hpp"
#include <iostream>

int main() {
    using namespace mechatronics;
    SecureTelemetryPipeline pipe(0x7F4A9C2E81B05D33ULL);
    auto pkt = pipe.encryptTelemetry(101, 12.5f, 44.2f, 1);

    float r = 0.0f, t = 0.0f;
    bool ok1 = pipe.decryptAndVerify(pkt, r, t);
    bool ok2 = pipe.decryptAndVerify(pkt, r, t);
    auto tampered = pkt; tampered.iv_nonce = 102; tampered.ciphertext[2] ^= 0x01;
    bool ok3 = pipe.decryptAndVerify(tampered, r, t);

    std::cout << "Valid Decrypt: " << ok1 << " (Roll=" << r << ", Temp=" << t << ") | Replay: " << ok2 << " | Tamper: " << ok3 << "\n";
    return (ok1 && !ok2 && !ok3) ? 0 : 1;
}
