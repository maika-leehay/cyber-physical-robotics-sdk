#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace mechatronics {

struct BoundingBox2D {
    float u_min;       // Left pixel coordinate
    float v_min;       // Top pixel coordinate
    float u_max;       // Right pixel coordinate
    float v_max;       // Bottom pixel coordinate
    float confidence;  // Model confidence score [0.0, 1.0]
    int   class_id;    // Target object class ID
    float depth_m;     // Aligned RGB-D depth at box centroid (meters)
};

struct Point3D {
    float x_m;         // Right axis in camera_optical_frame (meters)
    float y_m;         // Down axis in camera_optical_frame (meters)
    float z_m;         // Forward optical axis depth (meters)
    int   class_id;
    float confidence;
};

struct CameraIntrinsics {
    float fx = 615.0f; // Horizontal focal length (px, Intel RealSense D435i @ 640x480)
    float fy = 615.0f; // Vertical focal length (px)
    float cx = 320.0f; // Principal point X (px)
    float cy = 240.0f; // Principal point Y (px)
};

class SpatialVisionPipeline {
public:
    explicit SpatialVisionPipeline(CameraIntrinsics intrinsics, float conf_thresh = 0.50f, float iou_thresh = 0.45f)
        : K_(intrinsics), conf_thresh_(conf_thresh), iou_thresh_(iou_thresh) {}

    // Computes Intersection-over-Union (IoU) between two 2D bounding boxes
    static float computeIoU(const BoundingBox2D& a, const BoundingBox2D& b) {
        float inter_u1 = std::max(a.u_min, b.u_min);
        float inter_v1 = std::max(a.v_min, b.v_min);
        float inter_u2 = std::min(a.u_max, b.u_max);
        float inter_v2 = std::min(a.v_max, b.v_max);

        float inter_w = std::max(0.0f, inter_u2 - inter_u1);
        float inter_h = std::max(0.0f, inter_v2 - inter_v1);
        float inter_area = inter_w * inter_h;

        float area_a = std::max(0.0f, a.u_max - a.u_min) * std::max(0.0f, a.v_max - a.v_min);
        float area_b = std::max(0.0f, b.u_max - b.u_min) * std::max(0.0f, b.v_max - b.v_min);
        float union_area = area_a + area_b - inter_area;

        return (union_area > 1e-6f) ? (inter_area / union_area) : 0.0f;
    }

    // Filters raw ONNX tensor candidates via Non-Maximum Suppression (NMS)
    std::vector<BoundingBox2D> applyNMS(std::vector<BoundingBox2D> candidates) const {
        std::vector<BoundingBox2D> filtered;
        for (const auto& box : candidates) {
            if (box.confidence >= conf_thresh_) filtered.push_back(box);
        }
        std::sort(filtered.begin(), filtered.end(),
                  [](const BoundingBox2D& a, const BoundingBox2D& b) { return a.confidence > b.confidence; });

        std::vector<BoundingBox2D> kept;
        std::vector<bool> suppressed(filtered.size(), false);

        for (size_t i = 0; i < filtered.size(); ++i) {
            if (suppressed[i]) continue;
            kept.push_back(filtered[i]);
            for (size_t j = i + 1; j < filtered.size(); ++j) {
                if (!suppressed[j] && filtered[i].class_id == filtered[j].class_id) {
                    if (computeIoU(filtered[i], filtered[j]) > iou_thresh_) {
                        suppressed[j] = true;
                    }
                }
            }
        }
        return kept;
    }

    // Back-projects 2D bounding box centroid (u, v) + depth Z into 3D camera frame (X, Y, Z)
    Point3D deprojectPixelTo3D(const BoundingBox2D& box) const {
        float u_center = 0.5f * (box.u_min + box.u_min + (box.u_max - box.u_min));
        float v_center = 0.5f * (box.v_min + box.v_min + (box.v_max - box.v_min));

        float x_m = ((u_center - K_.cx) * box.depth_m) / K_.fx;
        float y_m = ((v_center - K_.cy) * box.depth_m) / K_.fy;
        return Point3D{ x_m, y_m, box.depth_m, box.class_id, box.confidence };
    }

private:
    CameraIntrinsics K_;
    float conf_thresh_;
    float iou_thresh_;
};

} // namespace mechatronics
