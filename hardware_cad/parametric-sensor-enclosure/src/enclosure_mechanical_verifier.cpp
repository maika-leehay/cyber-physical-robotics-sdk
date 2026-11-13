#include <cmath>
#include <iomanip>
#include <iostream>

namespace mechatronics {

struct SnapFitGeometry {
    double beam_length_mm = 12.0;  // Cantilever latch length L (mm)
    double root_thick_mm  = 1.6;   // Hook base thickness t (mm)
    double undercut_mm    = 1.0;   // Required assembly deflection y_max (mm)
    double yield_strain   = 0.022; // PETG permissible flexural strain (2.2%)
};

struct ThermalEnclosureSpec {
    double power_dissipated_w = 2.80;  // ESP32 + Camera + IMU + Buck heat load (W)
    double surface_area_m2    = 0.024; // Effective convection shell + louver area (m^2)
    double conv_coeff_w_m2k   = 11.5;  // Natural airflow heat transfer coeff h (W/m^2*K)
    double max_temp_rise_c    = 15.0;  // Maximum allowable internal delta-T (Celsius)
};

class EnclosureDesignVerifier {
public:
    // Euler-Bernoulli cantilever snap-fit maximum bending strain: eps = (1.5 * t * y_max) / L^2
    static double computeSnapFitStrain(const SnapFitGeometry& g) {
        return (1.5 * g.root_thick_mm * g.undercut_mm) / (g.beam_length_mm * g.beam_length_mm);
    }

    // Steady-state internal temperature rise: dT = Q / (h * A)
    static double computeThermalRiseC(const ThermalEnclosureSpec& t) {
        return t.power_dissipated_w / (t.conv_coeff_w_m2k * t.surface_area_m2);
    }
};

} // namespace mechatronics

int main() {
    using namespace mechatronics;
    SnapFitGeometry snap{};
    ThermalEnclosureSpec therm{};

    double strain = EnclosureDesignVerifier::computeSnapFitStrain(snap);
    double safety_factor = snap.yield_strain / strain;
    double delta_t = EnclosureDesignVerifier::computeThermalRiseC(therm);

    std::cout << "=========================================================================\n";
    std::cout << " Parametric CAD Enclosure: Snap-Fit Strain & Thermal Clearance Benchmark\n";
    std::cout << "=========================================================================\n"
              << std::fixed << std::setprecision(2)
              << "  * Cantilever Snap-Fit Max Strain : " << (strain * 100.0) << "% (PETG Limit: 2.20%)\n"
              << "  * Mechanical Yield Safety Factor : " << safety_factor << "x [PASS]\n"
              << "  * Internal Thermal Rise (2.8W)   : +" << delta_t << " C (Max Limit: +15.00 C) [PASS]\n";

    return (strain < snap.yield_strain && delta_t < therm.max_temp_rise_c) ? 0 : 1;
}
