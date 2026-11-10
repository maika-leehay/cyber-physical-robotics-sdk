#include "../include/spatial_vision_pipeline.hpp"
#include <iomanip>
#include <iostream>

int main() {
    using namespace mechatronics;
    CameraIntrinsics realsense_d435i{};
    SpatialVisionPipeline pipeline(realsense_d435i, 0.50f, 0.45f);

    std::vector<BoundingBox2D> raw_detections = {
        { 410.0f, 160.0f, 490.0f, 240.0f, 0.94f, 1, 0.650f }, // Target industrial gear
        { 414.0f, 162.0f, 488.0f, 238.0f, 0.81f, 1, 0.652f }, // Overlapping duplicate anchor
        { 180.0f, 260.0f, 240.0f, 320.0f, 0.89f, 2, 0.520f }, // Target PCB enclosure
        { 100.0f, 100.0f, 140.0f, 140.0f, 0.22f, 1, 0.800f }  // Low-confidence background noise
    };

    auto nms_boxes = pipeline.applyNMS(raw_detections);
    std::cout << "=========================================================================\n";
    std::cout << " 3D Spatial Vision Pipeline: ONNX Post-Processing & Pinhole Deprojection\n";
    std::cout << "=========================================================================\n";
    std::cout << "Raw Anchors: " << raw_detections.size() << " -> Post-NMS Valid Objects: " << nms_boxes.size() << "\n";

    for (const auto& box : nms_boxes) {
        Point3D pt = pipeline.deprojectPixelTo3D(box);
        std::cout << std::fixed << std::setprecision(4)
                  << "  * Class " << pt.class_id << " (conf=" << std::setprecision(2) << pt.confidence << ") -> "
                  << std::setprecision(4) << "3D Grasp Pose [X, Y, Z]: ["
                  << pt.x_m << " m, " << pt.y_m << " m, " << pt.z_m << " m]\n";
    }
    return (nms_boxes.size() == 2) ? 0 : 1;
}
