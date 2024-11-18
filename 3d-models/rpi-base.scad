// SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: CERN-OHL-S-2.0

padding = 5;

length = 85 + 2 * padding;
width = 56 + 2 * padding;

screw_padding = 3.5 + padding;
screw_distance_x = 58;
screw_distance_y = 49;

holder_padding_x = 3.5;
holder_padding_y = 15;

thickness = 2;

difference() {
    union() {
        cube([length, width, thickness]);
        translate([screw_padding, screw_padding, thickness])
            for(i=[0:1])
                for(j=[0:1])
                    translate([i * screw_distance_x, j * screw_distance_y, 0])
                        cylinder(h=4, d1=12, d2=6.8, $fn=20);
    }

    translate([screw_padding, screw_padding, 0])
        for(i=[0:1])
            for(j=[0:1])
                translate([i * screw_distance_x, j * screw_distance_y, 0]) {
                    cylinder(h=thickness + 3.3, d=5.6, $fn=20);
                    cylinder(h=thickness + 4, d=2.7, $fn=20);
                }

    translate([holder_padding_x, holder_padding_y, 0])
        cylinder(h=thickness, d=2.2, $fn=20);
    translate([length - holder_padding_x, holder_padding_y, 0])
        cylinder(h=thickness, d=2.2, $fn=20);
    translate([holder_padding_x, width - holder_padding_y, 0])
        cylinder(h=thickness, d=2.2, $fn=20);
    translate([length - holder_padding_x, width - holder_padding_y, 0])
        cylinder(h=thickness, d=2.2, $fn=20);
}
