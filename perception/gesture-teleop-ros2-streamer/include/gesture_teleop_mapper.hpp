#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mechatronics {

struct HandLandmarkFrame {
    double wrist_x;       // Landmark 0 normalized X [0.0, 1.0]
    double wrist_y;       // Landmark 0 normalized Y [0.0, 1.0]
    double thumb_tip_x;   // Landmark 4 normalized X
    double thumb_tip_y;   // Landmark 4 normalized Y
    double index_tip_x;   // Landmark 8 normalized X
    double index_tip_y;   // Landmark 8 normalized Y
    double confidence;    // MediaPipe hand presence confidence [0.0, 1.0]
    double frame_age_ms;  // Telemetry age since capture (ms)
};

struct TeleopTwistCmd {
    double linear_x_mps    = 0.0;
    double angular_z_rps   = 0.0;
    bool   deadman_engaged = false;
};

class GestureTeleopMapper {
public:
    GestureTeleopMapper(double deadzone = 0.08, double max_lin_mps = 0.60, double max_ang_rps = 1.20)
        : deadzone_(deadzone), max_lin_(max_lin_mps), max_ang_(max_ang_rps) {}

    TeleopTwistCmd mapLandmarksToTwist(const HandLandmarkFrame& f) const {
        if (f.confidence < 0.75 || f.frame_age_ms > 150.0) {
            return TeleopTwistCmd{ 0.0, 0.0, false };
        }
        double pinch_dist = std::hypot(f.index_tip_x - f.thumb_tip_x, f.index_tip_y - f.thumb_tip_y);
        if (pinch_dist < 0.06) {
            return TeleopTwistCmd{ 0.0, 0.0, false };
        }

        double dy = 0.50 - f.wrist_y;
        double dx = 0.50 - f.wrist_x;

        auto applyDeadzone = [&](double val, double max_out) -> double {
            if (std::abs(val) <= deadzone_) return 0.0;
            double sign = (val > 0.0) ? 1.0 : -1.0;
            double norm = (std::abs(val) - deadzone_) / (0.40 - deadzone_);
            return sign * max_out * std::clamp(norm, 0.0, 1.0);
        };

        return TeleopTwistCmd{ applyDeadzone(dy, max_lin_), applyDeadzone(dx, max_ang_), true };
    }

private:
    double deadzone_;
    double max_lin_;
    double max_ang_;
};

} // namespace mechatronics
