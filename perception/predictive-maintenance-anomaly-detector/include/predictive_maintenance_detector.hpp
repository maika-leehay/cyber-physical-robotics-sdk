#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace mechatronics {

enum class MotorHealthState : uint8_t {
    HEALTHY                  = 0,
    EARLY_BEARING_WEAR_WARN  = 1,
    CRITICAL_THERMAL_VIB_FLT = 2
};

struct TelemetryWindowFeatures {
    double rms_vib_g;       // Root-Mean-Square vibration amplitude (g)
    double peak_vib_g;      // Peak absolute vibration acceleration (g)
    double crest_factor;    // Peak / RMS ratio (early bearing spalling indicator)
    double temp_celsius;    // Stator winding temperature (Celsius)
};

struct AnomalyReport {
    TelemetryWindowFeatures features;
    double anomaly_score;   // Multivariate normalized Mahalanobis distance
    MotorHealthState state;
};

class PredictiveMaintenanceDetector {
public:
    PredictiveMaintenanceDetector(double warn_threshold = 3.0, double crit_threshold = 10.0)
        : warn_thresh_(warn_threshold), crit_thresh_(crit_threshold) {}

    // Extracts RMS, Peak, and Crest Factor from a window of high-rate accelerometer samples
    static TelemetryWindowFeatures extractFeatures(const std::vector<double>& accel_window_g, double temp_c) {
        if (accel_window_g.empty()) return {0.0, 0.0, 0.0, temp_c};

        double sum_sq = 0.0;
        double peak = 0.0;
        for (double a : accel_window_g) {
            double abs_a = std::abs(a);
            sum_sq += a * a;
            if (abs_a > peak) peak = abs_a;
        }
        double rms = std::sqrt(sum_sq / static_cast<double>(accel_window_g.size()));
        double crest = (rms > 1e-6) ? (peak / rms) : 0.0;
        return TelemetryWindowFeatures{ rms, peak, crest, temp_c };
    }

    // Evaluates multivariate anomaly score and classifies motor health state
    AnomalyReport evaluate(const std::vector<double>& accel_window_g, double temp_c) const {
        TelemetryWindowFeatures f = extractFeatures(accel_window_g, temp_c);

        double z_rms   = std::max(0.0, (f.rms_vib_g - mean_rms_) / std_rms_);
        double z_crest = std::max(0.0, (f.crest_factor - mean_crest_) / std_crest_);
        double z_temp  = std::max(0.0, (f.temp_celsius - mean_temp_) / std_temp_);

        // Euclidean norm in standardized feature space (diagonal Mahalanobis distance)
        double score = std::sqrt(z_rms * z_rms + z_crest * z_crest + z_temp * z_temp);

        MotorHealthState state = MotorHealthState::HEALTHY;
        if (score >= crit_thresh_) {
            state = MotorHealthState::CRITICAL_THERMAL_VIB_FLT;
        } else if (score >= warn_thresh_) {
            state = MotorHealthState::EARLY_BEARING_WEAR_WARN;
        }
        return AnomalyReport{ f, score, state };
    }

private:
    double warn_thresh_;
    double crit_thresh_;
    double mean_rms_   = 0.15, std_rms_   = 0.02;
    double mean_crest_ = 1.45, std_crest_ = 0.45;
    double mean_temp_  = 42.0, std_temp_  = 0.80;
};

} // namespace mechatronics
