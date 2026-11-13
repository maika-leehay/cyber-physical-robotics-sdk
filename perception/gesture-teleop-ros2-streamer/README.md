# Edge AI Hand-Gesture Teleoperation & ROS 2 Twist Safety Watchdog

C++17 human-robot interaction (HRI) controller translating 21-point **MediaPipe Hand Landmarks** into deadzone-filtered `geometry_msgs/Twist` velocity commands with a **Thumb-Index Pinch E-Stop** and **150 ms frame-drop watchdog**.

## 1. Resume / CV Bullet
* **Integrated 21-keypoint hand-tracking computer vision outputs with a C++17 ROS 2 Twist teleoperation controller, enforcing +/-8% radial tremor deadzones, thumb-index pinch emergency stops, and 150 ms frame-loss watchdogs.**
