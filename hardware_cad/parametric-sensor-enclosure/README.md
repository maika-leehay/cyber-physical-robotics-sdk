# Parametric 3D-Printable Multi-Sensor Enclosure (DFM, Snap-Fit Mechanics & Thermal Verification)

Parametric **OpenSCAD (`sensor_enclosure.scad`)** mechanical assembly and **C++17 structural/thermal verification suite** for an embedded robotics sensor payload (`ESP32/STM32`, `MPU-6050 IMU` centered at CG, and optical camera viewport).

## 1. Design for Manufacturing (DFM) & Tolerance Specifications
* **FDM Nozzle Compensation**: `+0.25 mm` XY mating lip clearance calibrated for `0.4 mm` nozzle extrusion width.
* **M3 Brass Heat-Set Insert Bosses**: `4.0 mm` ID / `6.4 mm` OD standoffs (`6.0 mm` depth) for knurled M3x5.7 brass inserts.
* **Euler-Bernoulli Snap-Fit Latch Design**: `L = 12.0 mm` cantilever length, `t = 1.6 mm` root thickness, and `1.0 mm` catch undercut yielding `1.67%` peak fiber strain (`1.32x` safety factor below `2.20%` PETG yield limit).
* **Natural Convection Louvers**: 12 lateral ventilation slots (`2.5 x 16.0 mm`) maintaining internal temperature rise at `+10.14 °C` under a continuous `2.80 W` thermal load.

## 2. Resume / CV Bullet
* **Designed a parametric 3D-printable multi-sensor hardware enclosure with M3 heat-set insert bosses and +0.25 mm FDM tolerances, validating cantilever snap-fit bending strain (1.67% vs. 2.20% PETG yield) and natural-convection thermal clearance (+10.14 °C at 2.8 W) in C++17.**
