// ============================================================================
// Parametric Multi-Sensor Embedded Enclosure (ESP32 + MPU-6050 + RGB-D Camera)
// Material: PETG / ABS | Nozzle: 0.4mm | XY Clearance Tolerance: +0.25mm
// ============================================================================
$fn = 64;

// Primary Parametric Dimensions (mm)
inner_length    = 85.0;
inner_width     = 58.0;
inner_height    = 32.0;
wall_thick      = 2.4;
fdm_clearance   = 0.25; // XY lip mating tolerance for 0.4mm FDM nozzle

// M3 Brass Heat-Set Insert Boss Parameters (Ruthex / CNC Kitchen M3x5.7)
boss_outer_d    = 6.4;
boss_inner_d    = 4.0;
boss_height     = 6.0;

// Cantilever Snap-Fit Latch Parameters (Verified: 1.67% strain < 2.2% PETG yield)
snap_length     = 12.0;
snap_thick      = 1.6;
snap_width      = 8.0;
snap_undercut   = 1.0;

module m3_heatset_boss(x, y) {
    translate([x, y, wall_thick])
    difference() {
        cylinder(d = boss_outer_d, h = boss_height);
        cylinder(d = boss_inner_d, h = boss_height + 0.2);
    }
}

module enclosure_base() {
    outer_l = inner_length + 2 * wall_thick;
    outer_w = inner_width  + 2 * wall_thick;
    outer_h = inner_height + wall_thick;

    difference() {
        // Outer protective shell
        cube([outer_l, outer_w, outer_h]);

        // Inner electronics cavity
        translate([wall_thick, wall_thick, wall_thick])
            cube([inner_length, inner_width, inner_height + 1.0]);

        // Thermal convection side louvers (6 slots per side, 2.5mm x 18mm)
        for (i = [0 : 5]) {
            translate([14.0 + i * 10.0, -0.5, 10.0])
                cube([2.5, outer_w + 1.0, 16.0]);
        }

        // Front RGB-D camera & LiDAR optical viewport cutout
        translate([outer_l - wall_thick - 0.5, outer_w / 2, outer_h / 2 + 2.0])
            rotate([0, 90, 0])
            cylinder(d = 14.0, h = wall_thick + 1.0);
    }

    // 4x Corner M3 PCB Standoffs (ESP32 Carrier Board)
    m3_heatset_boss(wall_thick + 6.0,                wall_thick + 6.0);
    m3_heatset_boss(wall_thick + inner_length - 6.0, wall_thick + 6.0);
    m3_heatset_boss(wall_thick + 6.0,                wall_thick + inner_width - 6.0);
    m3_heatset_boss(wall_thick + inner_length - 6.0, wall_thick + inner_width - 6.0);

    // 2x Center MPU-6050 IMU Standoffs (Aligned with Center of Gravity)
    m3_heatset_boss(outer_l / 2 - 10.0, outer_w / 2);
    m3_heatset_boss(outer_l / 2 + 10.0, outer_w / 2);
}

enclosure_base();
