#pragma once
#include <cstdint>
#include <cstdio>
#include <string>

namespace mechatronics {

class ComplementaryFilter {
public:
    explicit ComplementaryFilter(float alpha = 0.985f) : alpha_(alpha), angle_deg_(0.0f) {}

    float update(float accel_angle_deg, float gyro_rate_dps, float dt_sec) {
        angle_deg_ = alpha_ * (angle_deg_ + gyro_rate_dps * dt_sec) + (1.0f - alpha_) * accel_angle_deg;
        return angle_deg_;
    }

private:
    float alpha_;
    float angle_deg_;
};

class KalmanFilter1D {
public:
    KalmanFilter1D(float q_angle = 0.01f, float q_bias = 0.05f, float r_measure = 105.0f)
        : q_angle_(q_angle), q_bias_(q_bias), r_measure_(r_measure),
          angle_deg_(0.0f), bias_dps_(0.0f) {
        P_[0][0] = 1.0f; P_[0][1] = 0.0f;
        P_[1][0] = 0.0f; P_[1][1] = 4.0f;
    }

    float update(float accel_angle_deg, float gyro_rate_dps, float dt_sec) {
        // 1. Predict state: subtract estimated thermal drift bias from gyro rate
        float rate = gyro_rate_dps - bias_dps_;
        angle_deg_ += dt_sec * rate;

        // 2. Predict error covariance matrix P
        P_[0][0] += dt_sec * (dt_sec * P_[1][1] - P_[0][1] - P_[1][0] + q_angle_);
        P_[0][1] -= dt_sec * P_[1][1];
        P_[1][0] -= dt_sec * P_[1][1];
        P_[1][1] += q_bias_ * dt_sec;

        // 3. Compute innovation (measurement residual) and Kalman Gain K
        float innovation = accel_angle_deg - angle_deg_;
        float S = P_[0][0] + r_measure_;
        float K[2] = { P_[0][0] / S, P_[1][0] / S };

        // 4. Correct state estimate using accelerometer innovation
        angle_deg_ += K[0] * innovation;
        bias_dps_  += K[1] * innovation;

        // 5. Update error covariance matrix P
        float P00_temp = P_[0][0];
        float P01_temp = P_[0][1];
        P_[0][0] -= K[0] * P00_temp;
        P_[0][1] -= K[0] * P01_temp;
        P_[1][0] -= K[1] * P00_temp;
        P_[1][1] -= K[1] * P01_temp;

        return angle_deg_;
    }

    float estimatedBias() const { return bias_dps_; }

private:
    float q_angle_, q_bias_, r_measure_;
    float angle_deg_, bias_dps_;
    float P_[2][2];
};

inline uint8_t computeCRC8(const uint8_t* data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) {
            crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07) : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}

inline std::string formatUartFrame(uint32_t tick_ms, float raw_roll, float comp_roll, float kalman_roll, float bias_est) {
    char payload[128];
    int n = std::snprintf(payload, sizeof(payload), "IMU,%u,%.2f,%.2f,%.2f,%.3f",
                          tick_ms, raw_roll, comp_roll, kalman_roll, bias_est);
    uint8_t crc = computeCRC8(reinterpret_cast<const uint8_t*>(payload), static_cast<size_t>(n));
    char frame[160];
    std::snprintf(frame, sizeof(frame), "$%s*%02X", payload, crc);
    return std::string(frame);
}

} // namespace mechatronics
