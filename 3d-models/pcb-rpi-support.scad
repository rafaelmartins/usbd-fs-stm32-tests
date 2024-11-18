// SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: CERN-OHL-S-2.0

screw_distance = 58;
screw_base_height = 18.5;
screw_base_d = 5;
screw_base_hole_top_d = 2.5;
screw_base_hole_bottom_d = 2;

difference() {
    union() {
        cylinder(h=screw_base_height, d=screw_base_d, $fn=20);
        translate([0, -screw_base_d / 2, 0])
            cube([screw_distance, screw_base_d, screw_base_d]);
        translate([screw_distance, 0, 0])
            cylinder(h=screw_base_height, d=screw_base_d, $fn=20);
    }

    cylinder(h=screw_base_height / 2, d=screw_base_hole_bottom_d, $fn=20);
    translate([0, 0, screw_base_height / 2])
        cylinder(h=screw_base_height / 2, d=screw_base_hole_top_d, $fn=20);

    translate([screw_distance, 0, 0]) {
        cylinder(h=screw_base_height / 2, d=screw_base_hole_bottom_d, $fn=20);
        translate([0, 0, screw_base_height / 2])
            cylinder(h=screw_base_height / 2, d=screw_base_hole_top_d, $fn=20);
    }
}
