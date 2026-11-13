#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

namespace mechatronics {

class AdversarialVisionDefense {
public:
    // Applies Fast Gradient Sign Method (FGSM): x_adv = clip(x + eps * sign(grad), 0, 1)
    static std::vector<double> craftFgsmAttack(const std::vector<double>& clean_patch, const std::vector<double>& loss_grad, double eps) {
        std::vector<double> adv(clean_patch.size());
        for (size_t i = 0; i < clean_patch.size(); ++i) {
            double s = (loss_grad[i] >= 0.0) ? 1.0 : -1.0;
            adv[i] = std::clamp(clean_patch[i] + eps * s, 0.0, 1.0);
        }
        return adv;
    }

    // Detects adversarial perturbations via L1 Feature Squeezing + Cross-Modal LiDAR/RGB-D Depth Check
    static bool detectAdversarialPatch(const std::vector<double>& raw_logits, const std::vector<double>& squeezed_logits,
                                       double l1_threshold, double vision_depth_m, double lidar_depth_m) {
        double l1_diff = 0.0;
        for (size_t i = 0; i < raw_logits.size(); ++i) {
            l1_diff += std::abs(raw_logits[i] - squeezed_logits[i]);
        }
        bool feature_squeezing_alert = (l1_diff > l1_threshold);
        bool cross_modal_depth_alert = (std::abs(vision_depth_m - lidar_depth_m) > 0.25);
        return feature_squeezing_alert || cross_modal_depth_alert;
    }
};

} // namespace mechatronics
