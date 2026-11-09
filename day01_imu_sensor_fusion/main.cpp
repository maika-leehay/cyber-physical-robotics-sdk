#include "mpu6050_hal.hpp"
#include "sensor_fusion.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>

namespace mechatronics {

constexpr float PI_VAL     = 3.14159265358979323846f;
constexpr float RAD_TO_DEG = 57.29577951308232f;

class SimulatedMPU6050Bus : public II2CBus {
public:
    SimulatedMPU6050Bus() : rng_(42), accel_noise_(0.0f, 0.18f), gyro_noise_(0.0f, 0.35f) {}

    void setPhysicalState(float true_roll_deg, float true_roll_rate_dps, float gyro_bias_dps) {
        float roll_rad = true_roll_deg / RAD_TO_DEG;
        float ay_g = std::sin(roll_rad) + accel_noise_(rng_);
        float az_g = std::cos(roll_rad) + accel_noise_(rng_);
        float gx_dps = true_roll_rate_dps + gyro_bias_dps + gyro_noise_(rng_);

        encodeInt16(0, 0);
        encodeInt16(2, static_cast<int16_t>(ay_g * mpu6050_reg::ACCEL_LSB_PER_G));
        encodeInt16(4, static_cast<int16_t>(az_g * mpu6050_reg::ACCEL_LSB_PER_G));
        encodeInt16(6, static_cast<int16_t>((28.5f - 36.53f) * 340.0f));
        encodeInt16(8, static_cast<int16_t>(gx_dps * mpu6050_reg::GYRO_LSB_PER_DEG_SEC));
        encodeInt16(10, 0);
        encodeInt16(12, 0);
    }

    bool writeRegister(uint8_t devAddr, uint8_t regAddr, uint8_t value) override {
        if (devAddr != mpu6050_reg::DEV_ADDR) return false;
        regs_[regAddr] = value;
        return true;
    }

    bool readBurst(uint8_t devAddr, uint8_t startRegAddr, uint8_t* buffer, size_t length) override {
        if (devAddr != mpu6050_reg::DEV_ADDR) return false;
        if (startRegAddr == mpu6050_reg::WHO_AM_I && length == 1) {
            buffer[0] = mpu6050_reg::DEV_ADDR;
            return true;
        }
        if (startRegAddr == mpu6050_reg::ACCEL_XOUT_H && length == 14) {
            std::memcpy(buffer, burst_buf_, 14);
            return true;
        }
        return false;
    }

private:
    void encodeInt16(size_t offset, int16_t val) {
        burst_buf_[offset]     = static_cast<uint8_t>((static_cast<uint16_t>(val) >> 8) & 0xFF);
        burst_buf_[offset + 1] = static_cast<uint8_t>(static_cast<uint16_t>(val) & 0xFF);
    }

    std::mt19937 rng_;
    std::normal_distribution<float> accel_noise_;
    std::normal_distribution<float> gyro_noise_;
    uint8_t regs_[256] = {0};
    uint8_t burst_buf_[14] = {0};
};

} // namespace mechatronics

int main() {
    using namespace mechatronics;

    SimulatedMPU6050Bus i2c_bus;
    MPU6050Driver imu(i2c_bus);
    if (!imu.initialize()) return 1;

    ComplementaryFilter comp_filter(0.985f);
    KalmanFilter1D kalman_filter(0.01f, 0.05f, 105.0f);

    constexpr float dt_sec = 0.005f;        // 200 Hz loop
    constexpr int total_steps = 2000;       // 10 seconds
    constexpr float true_gyro_bias = 1.75f; // +1.75 deg/s drift

    double sq_err_raw = 0.0, sq_err_comp = 0.0, sq_err_kalman = 0.0;

    for (int step = 0; step < total_steps; ++step) {
        float t = step * dt_sec;
        float omega = 2.0f * PI_VAL * 0.4f;
        float true_roll_deg = 25.0f * std::sin(omega * t);
        float true_rate_dps = 25.0f * omega * std::cos(omega * t);

        i2c_bus.setPhysicalState(true_roll_deg, true_rate_dps, true_gyro_bias);
        RawImuSample sample{};
        imu.readSample(sample);

        float raw_roll    = std::atan2(sample.accel_g[1], sample.accel_g[2]) * RAD_TO_DEG;
        float comp_roll   = comp_filter.update(raw_roll, sample.gyro_dps[0], dt_sec);
        float kalman_roll = kalman_filter.update(raw_roll, sample.gyro_dps[0], dt_sec);

        sq_err_raw    += std::pow(raw_roll - true_roll_deg, 2);
        sq_err_comp   += std::pow(comp_roll - true_roll_deg, 2);
        sq_err_kalman += std::pow(kalman_roll - true_roll_deg, 2);

        if (step % 400 == 0 || step == total_steps - 1) {
            std::cout << formatUartFrame(step * 5, raw_roll, comp_roll, kalman_roll, kalman_filter.estimatedBias())
                      << " (True: " << std::fixed << std::setprecision(2) << true_roll_deg << " deg)\n";
        }
    }

    double rmse_raw    = std::sqrt(sq_err_raw / total_steps);
    double rmse_comp   = std::sqrt(sq_err_comp / total_steps);
    double rmse_kalman = std::sqrt(sq_err_kalman / total_steps);

    std::cout << "\n--- 200 Hz Benchmark Results ---\n" << std::fixed << std::setprecision(3)
              << "Raw Accel RMSE         : " << rmse_raw << " deg\n"
              << "Complementary RMSE     : " << rmse_comp << " deg\n"
              << "2-State Kalman RMSE    : " << rmse_kalman << " deg\n"
              << "Estimated Gyro Bias    : " << kalman_filter.estimatedBias() << " deg/s (True: 1.750)\n";
    return 0;
}
