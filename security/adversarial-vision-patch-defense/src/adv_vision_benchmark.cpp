#include "../include/adversarial_vision_defense.hpp"
#include <iostream>

int main() {
    using namespace mechatronics;
    bool clean_check = AdversarialVisionDefense::detectAdversarialPatch({0.91, 0.05, 0.04}, {0.89, 0.06, 0.05}, 0.35, 1.20, 1.22);
    bool adv_patch   = AdversarialVisionDefense::detectAdversarialPatch({0.12, 0.84, 0.04}, {0.86, 0.09, 0.05}, 0.35, 1.20, 1.21);
    bool phantom_obj = AdversarialVisionDefense::detectAdversarialPatch({0.88, 0.07, 0.05}, {0.85, 0.08, 0.07}, 0.35, 0.85, 3.40);
    std::cout << "Clean: " << clean_check << " | FGSM Patch Detected: " << adv_patch << " | 2D Poster Spoof Detected: " << phantom_obj << "\n";
    return (!clean_check && adv_patch && phantom_obj) ? 0 : 1;
}
