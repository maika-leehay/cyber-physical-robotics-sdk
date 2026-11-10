#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace mechatronics {

namespace mpu6050_reg {
constexpr uint8_t DEV_ADDR       = 0x68; // I2C slave address (AD0 = GND)
constexpr uint8_t SMPLRT_DIV     = 0x19; // Sample rate divider (200 Hz)
constexpr uint8_t CONFIG         = 0x1A; // Digital Low-Pass Filter (42 Hz)
constexpr uint8_t GYRO_CONFIG    = 0x1B; // +-250 deg/s full scale
constexpr uint8_t ACCEL_CONFIG   = 0x1C; // +-2g full scale
constexpr uint8_t ACCEL_XOUT_H   = 0x3B; // Start of 14-byte burst read
constexpr uint8_t PWR_MGMT_1     = 0x6B; // Wake-up & clock source register
constexpr uint8_t WHO_AM_I       = 0x75; // Identity register (returns 0x68)

constexpr float ACCEL_LSB_PER_G      = 16384.0f;
constexpr float GYRO_LSB_PER_DEG_SEC = 131.0f;
} // namespace mpu6050_reg

// Abstract I2C interface so the exact same driver runs on STM32, ESP32, or PC tests
class II2CBus {
public:
    virtual ~II2CBus() = default;
    virtual bool writeRegister(uint8_t devAddr, uint8_t regAddr, uint8_t value) = 0;
    virtual bool readBurst(uint8_t devAddr, uint8_t startRegAddr, uint8_t* buffer, size_t length) = 0;
};

struct RawImuSample {
    float accel_g[3];  // [ax, ay, az] in g
    float gyro_dps[3]; // [gx, gy, gz] in deg/s
    float temp_c;      // Die temperature in Celsius
};

class MPU6050Driver {
public:
    explicit MPU6050Driver(II2CBus& bus) : bus_(bus) {}

    bool initialize() {
        uint8_t whoAmI = 0;
        if (!bus_.readBurst(mpu6050_reg::DEV_ADDR, mpu6050_reg::WHO_AM_I, &whoAmI, 1) ||
            whoAmI != mpu6050_reg::DEV_ADDR) {
            return false;
        }
        bus_.writeRegister(mpu6050_reg::DEV_ADDR, mpu6050_reg::PWR_MGMT_1, 0x01);   // Wake up, PLL X-gyro clock
        bus_.writeRegister(mpu6050_reg::DEV_ADDR, mpu6050_reg::CONFIG, 0x03);       // DLPF ~42 Hz anti-aliasing
        bus_.writeRegister(mpu6050_reg::DEV_ADDR, mpu6050_reg::SMPLRT_DIV, 0x04);   // 1000Hz / (1 + 4) = 200 Hz
        bus_.writeRegister(mpu6050_reg::DEV_ADDR, mpu6050_reg::GYRO_CONFIG, 0x00);  // +-250 deg/s
        bus_.writeRegister(mpu6050_reg::DEV_ADDR, mpu6050_reg::ACCEL_CONFIG, 0x00); // +-2g
        return true;
    }

    bool readSample(RawImuSample& out) {
        uint8_t raw[14] = {0};
        if (!bus_.readBurst(mpu6050_reg::DEV_ADDR, mpu6050_reg::ACCEL_XOUT_H, raw, sizeof(raw))) {
            return false;
        }
        auto toInt16 = [](uint8_t hi, uint8_t lo) -> int16_t {
            return static_cast<int16_t>((static_cast<uint16_t>(hi) << 8) | lo);
        };

        out.accel_g[0]  = static_cast<float>(toInt16(raw[0],  raw[1]))  / mpu6050_reg::ACCEL_LSB_PER_G;
        out.accel_g[1]  = static_cast<float>(toInt16(raw[2],  raw[3]))  / mpu6050_reg::ACCEL_LSB_PER_G;
        out.accel_g[2]  = static_cast<float>(toInt16(raw[4],  raw[5]))  / mpu6050_reg::ACCEL_LSB_PER_G;
        out.temp_c      = (static_cast<float>(toInt16(raw[6], raw[7]))  / 340.0f) + 36.53f;
        out.gyro_dps[0] = static_cast<float>(toInt16(raw[8],  raw[9]))  / mpu6050_reg::GYRO_LSB_PER_DEG_SEC;
        out.gyro_dps[1] = static_cast<float>(toInt16(raw[10], raw[11])) / mpu6050_reg::GYRO_LSB_PER_DEG_SEC;
        out.gyro_dps[2] = static_cast<float>(toInt16(raw[12], raw[13])) / mpu6050_reg::GYRO_LSB_PER_DEG_SEC;
        return true;
    }

private:
    II2CBus& bus_;
};

} // namespace mechatronics
