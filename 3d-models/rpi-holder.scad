// SPDX-FileCopyrightText: 2024 Rafael G. Martins <rafael@rafaelmartins.eng.br>
// SPDX-License-Identifier: CERN-OHL-S-2.0

difference() {
    union() {
        cube([4, 2.2, 4]);
        translate([0, 2.2, 0])
            cube([8, 4, 4]);
    }

    translate([2, 0, 2])
        rotate([-90, 0, 0])
            cylinder(h=6, d=1.8, $fn=20);
}
