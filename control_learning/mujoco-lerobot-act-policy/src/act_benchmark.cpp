#include "../include/sim2real_act_evaluator.hpp"
#include <iomanip>
#include <iostream>

int main() {
    using namespace mechatronics;
    SimToRealPolicyEvaluator evaluator(/*action_chunk_horizon=*/ 16);
    auto perturbed_envs = SimToRealPolicyEvaluator::sampleRandomizedDomains(50);

    double sum_rmse_baseline = 0.0, sum_rmse_dr = 0.0;
    int success_baseline = 0, success_dr = 0; // Grasp tolerance < 6.0 mm

    for (const auto& env : perturbed_envs) {
        double err_base = evaluator.evaluateRolloutRmseMm(env, false);
        double err_dr   = evaluator.evaluateRolloutRmseMm(env, true);
        sum_rmse_baseline += err_base;
        sum_rmse_dr       += err_dr;
        if (err_base < 6.0) ++success_baseline;
        if (err_dr   < 6.0) ++success_dr;
    }

    double avg_base = sum_rmse_baseline / perturbed_envs.size();
    double avg_dr   = sum_rmse_dr / perturbed_envs.size();

    std::cout << "=========================================================================\n";
    std::cout << " Sim-to-Real ACT Manipulation Policy & Domain Randomization Benchmark\n";
    std::cout << "=========================================================================\n"
              << std::fixed << std::setprecision(2)
              << "  * Unrandomized Policy Tracking RMSE (50 perturbed rollouts): " << avg_base << " mm\n"
              << "  * Domain-Randomized ACT Policy RMSE (k=16 Temporal Chunk)  : " << avg_dr   << " mm\n"
              << "  * Sim-to-Real Grasp Success Rate (<6mm tolerance)          : "
              << (success_baseline * 2) << "% -> " << (success_dr * 2) << "%\n";

    return (avg_dr < 5.0 && success_dr > success_baseline) ? 0 : 1;
}
