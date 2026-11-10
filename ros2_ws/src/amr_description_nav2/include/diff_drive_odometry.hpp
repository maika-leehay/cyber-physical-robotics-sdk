#pragma once
#include <cmath>
#include <cstdint>

namespace mechatronics {

struct Pose2D {
    double x_m   = 0.0; // Global X position in meters (odom frame)
    double y_m   = 0.0; // Global Y position in meters (odom frame)
    double yaw_r = 0.0; // Heading angle in radians [-pi, +pi]
};

struct Quaternion {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double w = 1.0;
};

// Converts planar yaw (radians) to unit Quaternion for ROS 2 geometry_msgs/TransformStamped
inline Quaternion yawToQuaternion(double yaw_rad) {
    return Quaternion{ 0.0, 0.0, std::sin(yaw_rad * 0.5), std::cos(yaw_rad * 0.5) };
}

class DiffDriveOdometry {
public:
    DiffDriveOdometry(double wheel_radius_m, double track_width_m, int32_t ticks_per_rev)
        : wheel_radius_(wheel_radius_m),
          track_width_(track_width_m),
          meters_per_tick_((2.0 * 3.14159265358979323846 * wheel_radius_m) / static_cast<double>(ticks_per_rev)) {}

    // Updates pose using 2nd-order Runge-Kutta (exact midpoint arc) integration
    void updateRk2(int32_t delta_left_ticks, int32_t delta_right_ticks, double dt_sec) {
        double ds_left  = static_cast<double>(delta_left_ticks)  * meters_per_tick_;
        double ds_right = static_cast<double>(delta_right_ticks) * meters_per_tick_;

        double ds_center = 0.5 * (ds_right + ds_left);
        double d_yaw     = (ds_right - ds_left) / track_width_;

        // Midpoint heading eliminates 1st-order tangential chord drift during turns
        double mid_yaw = pose_.yaw_r + 0.5 * d_yaw;
        pose_.x_m   += ds_center * std::cos(mid_yaw);
        pose_.y_m   += ds_center * std::sin(mid_yaw);
        pose_.yaw_r  = normalizeAngle(pose_.yaw_r + d_yaw);

        linear_vel_mps_  = (dt_sec > 0.0) ? (ds_center / dt_sec) : 0.0;
        angular_vel_rps_ = (dt_sec > 0.0) ? (d_yaw / dt_sec)     : 0.0;
    }

    const Pose2D& getPose() const { return pose_; }
    double getLinearVelocity() const { return linear_vel_mps_; }
    double getAngularVelocity() const { return angular_vel_rps_; }

private:
    static double normalizeAngle(double angle_rad) {
        constexpr double PI = 3.14159265358979323846;
        while (angle_rad >  PI) angle_rad -= 2.0 * PI;
        while (angle_rad < -PI) angle_rad += 2.0 * PI;
        return angle_rad;
    }

    double wheel_radius_;
    double track_width_;
    double meters_per_tick_;
    Pose2D pose_{};
    double linear_vel_mps_  = 0.0;
    double angular_vel_rps_ = 0.0;
};

} // namespace mechatronics
