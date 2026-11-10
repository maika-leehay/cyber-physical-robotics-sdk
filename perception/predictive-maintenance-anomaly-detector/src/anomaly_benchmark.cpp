#include "../include/predictive_maintenance_detector.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>

int main() {
    using namespace mechatronics;
    constexpr double PI = 3.14159265358979323846;

    PredictiveMaintenanceDetector detector(/*warn=*/ 3.0, /*crit=*/ 10.0);

    std::vector<double> win_healthy(128), win_spalling(128), win_critical(128);
    for (int i = 0; i < 128; ++i) {
        double phase = 2.0 * PI * i / 16.0;
        win_healthy[i]  = 0.20 * std::sin(phase);
        win_spalling[i] = 0.21 * std::sin(phase) + ((i % 32 == 0) ? 0.65 : 0.0);
        win_critical[i] = 0.68 * std::sin(phase) + ((i % 16 == 0) ? 0.90 : 0.0);
    }

    auto r1 = detector.evaluate(win_healthy,  42.1);
    auto r2 = detector.evaluate(win_spalling, 43.2);
    auto r3 = detector.evaluate(win_critical, 58.5);

    auto printReport = [](const char* label, const AnomalyReport& r) {
        const char* state_str = (r.state == MotorHealthState::HEALTHY) ? "HEALTHY" :
                                (r.state == MotorHealthState::EARLY_BEARING_WEAR_WARN) ? "EARLY_BEARING_WARN" : "CRITICAL_FAULT";
        std::cout << std::fixed << std::setprecision(2)
                  << "  * " << label << " -> RMS: " << r.features.rms_vib_g << "g | Crest: "
                  << r.features.crest_factor << " | Temp: " << r.features.temp_celsius
                  << "C | Score: " << r.anomaly_score << " [" << state_str << "]\n";
    };

    std::cout << "=========================================================================\n";
    std::cout << " Edge AI Predictive Maintenance: Crest Factor & Mahalanobis Anomaly Node\n";
    std::cout << "=========================================================================\n";
    printReport("Regime 1 (Nominal)  ", r1);
    printReport("Regime 2 (Spalling) ", r2);
    printReport("Regime 3 (Runaway)  ", r3);

    return (r1.state == MotorHealthState::HEALTHY &&
            r2.state == MotorHealthState::EARLY_BEARING_WEAR_WARN &&
            r3.state == MotorHealthState::CRITICAL_THERMAL_VIB_FLT) ? 0 : 1;
}
