use <../modely/boxes.scad>
use <../modely/esp32.scad>
use <../modely/charger-module.scad>
use <../modely/flexbatter.scad>
use <../modely/buttons.scad>
use <../modely/mfrc.scad>

//DEBUG = true;
DEBUG = false;

fatness = 2;
inset = .2;

display_size_top = [164, 99.8, 1.5];
display_size_mid = [165.5, 101.7, 5];
display_size_board = [190, 105, 1.55];
display_size_bottom = [156, 101.7, 5];

display_hole_pos = [5, 12.5];
display_hole_d = 3;

onboard_sd_size = [14.6, 15, 1.5];
onboard_sd_pos = [1, 66];

sd_conn_size = [16, 22];
sd_size = [38, 30, 3.9];
sd_hold_x = 15;
sd_hold_size = 6;

pcb_size = [60, 40, 1.6];
rtc_size = [27, 28, 9.56];

batt_holder_size = [77.5, 22.5, 17.2];
esp32_size = ESP32_size();
mfrc_shaft_size = Shaft_size_outter();
charger_size = Charger_size();

height = 30;

// -------------------------------------

display_height_total = display_size_bottom.z + display_size_board.z + display_size_mid.z + display_size_top.z;

box_x = 2 + display_size_board.x + 2 * inset + sd_conn_size.x * 2;
box_y = 0.7 + display_size_board.y + 2 * inset + pcb_size.y;
box_z = 30;

box_inner = [box_x, box_y, box_z];
box = [box_inner.x + 2 * fatness, box_inner.y + 2 * fatness, box_inner.z + fatness];

echo("Main - Box size ", box.x, " x ", box.y, " x ", box.z);
echo("Main - Inner box size ", box_inner.x, " x ", box_inner.y, " x ", box_inner.z);

// -------------------------------------

module Pcb() {
    esp_pos = [8.2, 8, 11.2];
    rtc_pos = [51, esp_pos.y + 0.2, 0];

    color("gray") cube(pcb_size);
    color("red") translate([12, esp_pos.y, pcb_size.z]) cube([38.5, 2.4, esp_pos.z]);
    color("red") translate([12, esp_pos.y + esp32_size.y - 2.4, pcb_size.z]) cube([38.5, 2.4, esp_pos.z]);
    color("black") translate([esp_pos.x, esp_pos.y, esp_pos.z + pcb_size.z]) ESP32();
    color("black") translate([rtc_pos.x, rtc_pos.y, rtc_pos.z + pcb_size.z]) cube(rtc_size);
}

module Display() {
    translate([(display_size_board.x - display_size_bottom.x) / 2, (display_size_board.y - display_size_bottom.y) / 2])
        color("black") cube(display_size_bottom);

    translate([0, 0, display_size_bottom.z]) {
        difference() {
            color("green") cube(display_size_board);

            for (col = [- 1:2:1]) for (row = [- 1:2:1]) {
                x = display_size_board.x / 2 + col * (display_size_board.x / 2 - display_hole_pos.x);
                y = display_size_board.y / 2 + row * (display_size_board.y / 2 - display_hole_pos.y);
                translate([x, y]) cylinder(d = display_hole_d, h = display_size_board.z + .02, $fn = 20);
            }
        }


        h = display_size_board.z;
        translate([(display_size_board.x - display_size_mid.x) / 2, (display_size_board.y - display_size_mid.y) / 2, h])
            color("gray") cube(display_size_mid);
        h2 = display_size_board.z + display_size_mid.z;
        translate([(display_size_board.x - display_size_top.x) / 2, (display_size_board.y - display_size_top.y) / 2, h2])
            color("black") cube(display_size_top);

        // SD slot
        color("red") translate([onboard_sd_pos.x, onboard_sd_pos.y, - onboard_sd_size.z]) cube(onboard_sd_size);

        // SD connector
        color("orange") translate([- sd_conn_size.x, onboard_sd_pos.y + (onboard_sd_size.y - sd_conn_size.y) / 2, - 1])
            cube([sd_conn_size.x, sd_conn_size.y, 1]);
    }
}

module BatteryHolder() {
    translate([11.45, 69.25]) rotate([0, 0, - 90]) {
        flexbatter(n = 6, l = 66.8, d = 18.4, hf = 0.75, shd = 3, eps = 0.28);
    }
}

// -------------------------------------

// main
module Main() {
    union() {
        difference() {
            cube(box);
            translate([fatness, fatness, fatness]) color("blue") cube([box_inner.x, box_inner.y, 100]);

            // DEBUG
            translate([fatness, - .01, fatness]) cube([box_inner.x, fatness + .02, 100]); // front wall

            translate([fatness, fatness, fatness]) {
                // charger hole
                translate([- fatness - .01, 85.2, .2]) {
                    cube([fatness + .02, 11, 5]);
                }

                // switch
                translate([- fatness - .01, 70, 8]) {
                    rotate([0, 90])cylinder(d = switch_hole_dia(), h = fatness + .02, $fn = 30);
                }
            }
        }

        translate([fatness, fatness, fatness - .01]) {
            display_z = box_inner.z - (display_height_total - display_size_top.z);
            translate([(box_inner.x - display_size_board.x) / 2, - inset + box_inner.y - display_size_board.y]) {
                //                if (DEBUG) translate([0, 0, display_z]) Display();

                // display supports
                for (col = [- 1:2:1])
                for (row = [- 1:2:1]) {
                    x = display_size_board.x / 2 + col * (display_size_board.x / 2 - display_hole_pos.x);
                    y = display_size_board.y / 2 + row * (display_size_board.y / 2 - display_hole_pos.y);
                    translate([x, y]) difference() {
                        h = display_z + display_size_bottom.z;
                        color("red") cylinder(d = display_hole_d + 6, h = h, $fn = 20);
                        color("blue") cylinder(d = display_hole_d, h = 100, $fn = 20);
                    }
                }
            }


            // PCB
            translate([- 1, 0, 0]) {
                if (DEBUG) translate([1 + inset, 1 + inset]) Pcb();

                // board bed
                difference() {
                    cube([pcb_size.x + 2 * (1 + inset), pcb_size.y + 2 * (1 + inset), 6]);
                    translate([1, 1]) cube([pcb_size.x + 2 * inset, pcb_size.y + 2 * inset, 100]);

                    // saving space
                    translate([5, - .01]) cube([52, 1 + .02, 100]);
                    translate([pcb_size.x + 1, 9]) cube([100, rtc_size.y + 1, 100]);
                }
            }

            // MFRC
            translate([(box.x - MFRC_board_size().x) / 2 - 1, - 1, - .01]) {
                if (DEBUG) translate([1 + inset, MFRC_board_size().y + 1 + inset, box_inner.z + .01]) rotate([180]) MFRC_board();

                difference() {
                    outter = [MFRC_board_size().x + 2 * inset + 2, MFRC_board_size().y + 2 * inset + 2, box_inner.z];
                    cube(outter);
                    translate([1, 1]) cube([MFRC_board_size().x + 2 * inset, MFRC_board_size().y + 2 * inset, 100]);

                    translate([- .01, 18, 22]) cube([2, 15, 100]);
                    translate([- .01, 5]) cube([100, outter.y - 2 * 5, 15]);
                    translate([10, - .01]) cube([outter.x - 20, 100, 20]);

                    // debug MRFC stand:
                    // translate([1, -1]) cube([MFRC_board_size().x + 2 * inset, 100, 100]);
                }

                //supports
                translate([1, 1]) {
                    translate([18, 5]) difference() {
                        outter = [MFRC_board_size().x - 2 * 18, MFRC_board_size().y - 2 * 5, box_inner.z - MFRC_board_size().z - inset];
                        cube(outter);
                        translate([1.5, 1.5]) cube([outter.x - 2 * 1.5, outter.y - 2 * 1.5, 100]);

                    }
                }
            }

            // battery holder
            translate([48.5, 45, - 1.85]) {
                BatteryHolder();
            }

            // switch
            translate([15.5, 70, 8]) {
                if (DEBUG) rotate([0, - 90]) Switch(fatness + inset);
            }

            // charger
            translate([inset, 82, 0]) {
                if (DEBUG)  translate([28, 0, 1]) rotate([90, 180, - 90]) Charger();
                translate([charger_size.z - 5, - 1 - inset]) difference() {
                    cube([5 + 1 + inset, charger_size.x + 2 * (1 + inset), 2]);
                    translate([- .01, 1]) cube([5 + inset, charger_size.x + 2 * inset, 100]);
                }
            }
        }
    }
}

difference() {
    Main();


    // /* PRINTING SETTINGS: */
    //
    // // left wall:
    //
    // // PCB
    // translate([- .01, - .01, 10]) cube([fatness + .02, 60, 100]);
    // // switch, charger
    // translate([- .01, 55, 20]) cube([fatness + .02, 100, 100]);
    // translate([- .01, 85, 10]) cube([fatness + .02, 100, 100]);
    // // rest
    // translate([- .01, 110, fatness]) cube([fatness + .02, 100, 100]);
    //
    //
    // // back wall
    // translate([- .01, box.y - fatness - .01, fatness]) cube([box.x + .02, fatness + .02, 100]);
    //
    //
    // // right wall
    // translate([box.x - fatness - .01, - .01, fatness]) cube([fatness + .02, box.y + .02, 100]);
}
