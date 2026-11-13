#include "../include/gesture_teleop_mapper.hpp"
#include <iomanip>
#include <iostream>

int main() {
    using namespace mechatronics;
    GestureTeleopMapper mapper(0.08, 0.60, 1.20);

    auto c1 = mapper.mapLandmarksToTwist({ 0.26, 0.18, 0.20, 0.15, 0.35, 0.05, 0.96, 12.0 });
    auto c2 = mapper.mapLandmarksToTwist({ 0.52, 0.48, 0.45, 0.45, 0.60, 0.35, 0.94, 15.0 });
    auto c3 = mapper.mapLandmarksToTwist({ 0.30, 0.20, 0.31, 0.21, 0.32, 0.22, 0.95, 14.0 });
    auto c4 = mapper.mapLandmarksToTwist({ 0.26, 0.18, 0.20, 0.15, 0.35, 0.05, 0.96, 220.0 });

    std::cout << "=========================================================================\n";
    std::cout << " Edge AI Hand-Gesture Teleop & ROS 2 Twist Watchdog Verification\n";
    std::cout << "=========================================================================\n"
              << std::fixed << std::setprecision(2)
              << "  * Test 1 (Forward-Left Open Palm) : v=" << c1.linear_x_mps << " m/s, w=" << c1.angular_z_rps << " rad/s [ACTIVE]\n"
              << "  * Test 2 (Hand Tremor in Deadzone): v=" << c2.linear_x_mps << " m/s, w=" << c2.angular_z_rps << " rad/s [SUPPRESSED]\n"
              << "  * Test 3 (Thumb-Index Pinch EStop): v=" << c3.linear_x_mps << " m/s, w=" << c3.angular_z_rps << " rad/s [PINCH_HALT]\n"
              << "  * Test 4 (220ms Frame Drop Timeout): v=" << c4.linear_x_mps << " m/s, w=" << c4.angular_z_rps << " rad/s [WATCHDOG_HALT]\n";

    return (c1.linear_x_mps > 0.40 && c2.linear_x_mps == 0.0 && !c3.deadman_engaged && !c4.deadman_engaged) ? 0 : 1;
}
