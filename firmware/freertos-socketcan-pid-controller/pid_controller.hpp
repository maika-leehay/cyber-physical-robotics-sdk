#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mechatronics {

// ============================================================================
// 1. 4x Quadrature Encoder State Machine (Constant-Time Lookup Table)
// ============================================================================
class QuadratureEncoder {
public:
    explicit QuadratureEncoder(int32_t counts_per_rev = 2048)
        : cpr_(counts_per_rev), position_ticks_(0), prev_state_(0) {}

    // Updates encoder position on every pin change interrupt (Channel A & B)
    void update(bool pin_a, bool pin_b) {
        uint8_t curr_state = (pin_a ? 2 : 0) | (pin_b ? 1 : 0);
        uint8_t transition = (prev_state_ << 2) | curr_state;

        // 4x transition table: returns +1 (CW), -1 (CCW), or 0 (stationary/invalid)
        static constexpr int8_t LUT[16] = {
             0, -1,  1,  0,
             1,  0,  0, -1,
            -1,  0,  0,  1,
             0,  1, -1,  0
        };

        position_ticks_ += LUT[transition & 0x0F];
        prev_state_ = curr_state;
    }

    int32_t getTicks() const { return position_ticks_; }

    float getAngleDegrees() const {
        return (static_cast<float>(position_ticks_) / static_cast<float>(cpr_)) * 360.0f;
    }

    void reset() {
        position_ticks_ = 0;
        prev_state_ = 0;
    }

private:
    int32_t cpr_;
    int32_t position_ticks_;
    uint8_t prev_state_;
};

// ============================================================================
// 2. Industrial PID Controller with Anti-Windup Clamping & Derivative Filtering
// ============================================================================
class PIDController {
public:
    struct Gains {
        float kp = 2.5f;   // Proportional gain (torque per degree error)
        float ki = 0.8f;   // Integral gain (eliminates steady-state load friction)
        float kd = 0.08f;  // Derivative gain (damps oscillation and overshoot)
    };

    explicit PIDController(Gains gains, float output_limit = 100.0f)
        : gains_(gains), max_output_(output_limit), integrator_(0.0f),
          prev_measurement_(0.0f), d_filtered_(0.0f) {}

    /**
     * @brief Computes control effort u(t) at 1 kHz (dt = 0.001s).
     * @param setpoint Target joint angle (degrees).
     * @param measurement Measured joint angle from encoder (degrees).
     * @param dt_sec Loop period in seconds (0.001 for 1 kHz).
     * @return Clamped control output in [-max_output, +max_output] (% PWM).
     */
    float update(float setpoint, float measurement, float dt_sec) {
        float error = setpoint - measurement;

        // 1. Proportional Term
        float p_term = gains_.kp * error;

        // 2. Derivative on Measurement (avoids derivative kick on setpoint step)
        float d_raw = -(measurement - prev_measurement_) / dt_sec;
        // 1st-order low-pass filter (cutoff ~50 Hz at 1 kHz sample rate)
        constexpr float alpha = 0.75f;
        d_filtered_ = alpha * d_filtered_ + (1.0f - alpha) * d_raw;
        float d_term = gains_.kd * d_filtered_;
        prev_measurement_ = measurement;

        // 3. Tentative Output (PD only)
        float unsaturated_output = p_term + integrator_ + d_term;

        // 4. Integrator Update with Anti-Windup Clamping
        // Only integrate if output is NOT saturated, or if error helps pull out of saturation
        bool is_saturated = (unsaturated_output > max_output_) || (unsaturated_output < -max_output_);
        bool same_sign = (unsaturated_output * error) > 0.0f;

        if (!is_saturated || !same_sign) {
            integrator_ += gains_.ki * error * dt_sec;
            // Strict clamping on integrator accumulator
            integrator_ = std::clamp(integrator_, -max_output_, max_output_);
        }

        // 5. Final Output Saturation
        return std::clamp(p_term + integrator_ + d_term, -max_output_, max_output_);
    }

    void reset() {
        integrator_ = 0.0f;
        prev_measurement_ = 0.0f;
        d_filtered_ = 0.0f;
    }

private:
    Gains gains_;
    float max_output_;
    float integrator_;
    float prev_measurement_;
    float d_filtered_;
};

} // namespace mechatronics
