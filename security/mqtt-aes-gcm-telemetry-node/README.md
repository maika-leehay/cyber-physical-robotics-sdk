# Secure Edge-to-Cloud IoT Telemetry Pipeline (AEAD Encryption & IV Anti-Replay)

C++17 authenticated encryption and integrity verification pipeline for microcontroller MQTT telemetry, enforcing **strictly increasing 64-bit IV nonces** and **Verify-Then-Decrypt MAC tags** to reject replayed or bit-flipped packets.

## 1. Resume / CV Bullet
* **Designed a secure edge-to-cloud IoT telemetry pipeline in C++17 utilizing stream-cipher AEAD framing, 64-bit monotonic IV nonces, and Verify-Then-Decrypt integrity tags to block packet replay and ciphertext tampering.**
