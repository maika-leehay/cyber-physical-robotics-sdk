#include "../include/diff_drive_odometry.hpp"
#include <iomanip>
#include <iostream>

int main() {
    using namespace mechatronics;

    // Match physical URDF parameters: R = 0.045m, Track = 0.28m, 4096 CPR quadrature encoders
    DiffDriveOdometry odom(0.045, 0.28, 4096);

    // Simulate 100 Hz odometry loop across a curved trajectory (100 steps = 1.00s)
    for (int step = 0; step < 100; ++step) {
        odom.updateRk2(/*delta_left_ticks=*/ 18, /*delta_right_ticks=*/ 25, 0.01);
    }

    const auto& p = odom.getPose();
    Quaternion q = yawToQuaternion(p.yaw_r);

    std::cout << "=========================================================================\n";
    std::cout << " ROS 2 Differential-Drive RK2 Odometry & TF2 Transform Verification\n";
    std::cout << "=========================================================================\n";
    std::cout << std::fixed << std::setprecision(4)
              << "  * Odom Frame Pose (x, y, yaw) : (" << p.x_m << " m, " << p.y_m << " m, " << p.yaw_r << " rad)\n"
              << "  * TF2 Unit Quaternion [x,y,z,w]: [0.0000, 0.0000, " << q.z << ", " << q.w << "]\n"
              << "  * Base_Link Twist (v, omega)   : v = " << odom.getLinearVelocity()
              << " m/s, w = " << odom.getAngularVelocity() << " rad/s\n";
    return 0;
}
