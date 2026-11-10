#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mechatronics {

struct TwistCommand {
    double linear_x_mps  = 0.0;   // Forward velocity v_x (m/s) on /cmd_vel
    double angular_z_rps = 0.0;   // Yaw rate omega_z (rad/s) on /cmd_vel
    bool   safety_stop   = false; // ISO 10218-1:2025 monitored standstill flag
};

class LidarObstacleAvoidance {
public:
    LidarObstacleAvoidance(double stop_dist_m = 0.35, double slow_dist_m = 0.90, double max_speed_mps = 0.50)
        : stop_dist_(stop_dist_m), slow_dist_(slow_dist_m), max_speed_(max_speed_mps) {}

    /**
     * @brief Computes collision-free Twist (/cmd_vel) from 3 front LiDAR sectors:
     *        Right [-60..-20 deg], Front [-20..+20 deg], Left [+20..+60 deg]
     */
    TwistCommand computeVelocity(double min_front_m, double min_left_m, double min_right_m) const {
        TwistCommand cmd{};

        // 1. Critical Zone: Zero forward velocity and pivot toward open lateral sector
        if (min_front_m <= stop_dist_) {
            cmd.linear_x_mps  = 0.0;
            cmd.angular_z_rps = (min_left_m >= min_right_m) ? 0.85 : -0.85;
            cmd.safety_stop   = true;
            return cmd;
        }

        // 2. Caution Zone: Proportional linear deceleration + smooth obstacle steering
        if (min_front_m < slow_dist_) {
            double scale = (min_front_m - stop_dist_) / (slow_dist_ - stop_dist_);
            cmd.linear_x_mps  = max_speed_ * std::clamp(scale, 0.15, 1.0);
            cmd.angular_z_rps = (min_left_m >= min_right_m) ? 0.45 : -0.45;
        } else {
            // 3. Clear Corridor: Cruise at nominal speed with corridor-centering P-control
            cmd.linear_x_mps  = max_speed_;
            double lateral_err = std::clamp(min_left_m - min_right_m, -1.0, 1.0);
            cmd.angular_z_rps  = 0.25 * lateral_err;
        }
        return cmd;
    }

private:
    double stop_dist_;
    double slow_dist_;
    double max_speed_;
};

} // namespace mechatronics
