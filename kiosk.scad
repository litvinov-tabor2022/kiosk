use <../modely/esp32.scad>
use <../modely/charger-module.scad>
use <../modely/buttons.scad>
use <../modely/mfrc.scad>

//DEBUG = true;
DEBUG = false;

fatness = 2;
inset = .2;

display_size_top = [164, 99.8, 1.5];
display_size_mid = [165.5, 101.7, 5];
display_size_board = [190, 105.55, 1.55];
display_size_bottom = [156, 101.7, 5];

display_hole_pos = [
        [4.90, 12.80], // top left
        [4.90, 12.80], // bottom left
        [4.75, 12.75], // top right
        [4.75, 12.75], // bottom right
    ];
display_hole_d = 3;

onboard_sd_size = [14.6, 15, 1.5];
onboard_sd_pos = [1, 66];

sd_conn_size = [16, 22];
sd_size = [38, 30, 3.9];
sd_hold_x = 15.7;
sd_hold_size = [5.7, 4.5, 1];

pcb_size = [60, 40, 1.6];
rtc_size = [27, 28, 9.56];

battery_length = 66;
battery_dia = 19;
batt_holder_size = [battery_dia + 2 * 1, 66 + 2 * 1, battery_dia + 1];

esp32_size = ESP32_size();
mfrc_shaft_size = Shaft_size_outter();
charger_size = Charger_size();


y_coef = - 5.55;

switch_y = 54.5 - y_coef;
charger_y = 74.8 - y_coef;
sdslot_y = 98 - y_coef;

switch_z = 10;

height = 30;

batts = 6;

// -------------------------------------

display_height_total = display_size_bottom.z + display_size_board.z + display_size_mid.z + display_size_top.z;

box_x = 2 + display_size_board.x + 2 * inset + sd_conn_size.x * 2;
box_y = 8 + display_size_board.y + 2 * inset + pcb_size.y;
box_z = 35;

box_inner = [box_x, box_y, box_z];
box = [box_inner.x + 2 * fatness, box_inner.y + 2 * fatness, box_inner.z + fatness];

echo("Main - Box size ", box.x, " x ", box.y, " x ", box.z);
echo("Main - Inner box size ", box_inner.x, " x ", box_inner.y, " x ", box_inner.z);

display_z = box_inner.z - (display_height_total - display_size_top.z);

echo("Display Z:", display_z);

disp_pos = [(box_inner.x - display_size_board.x) / 2, - inset + box_inner.y - display_size_board.y - 1.5];
batt_holder_pos = [(box_inner.x - (batt_holder_size.x * batts - (batts - 1))) / 2, 60, 0];

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
    module Hole(x, y) {
        translate([x, y, - .01]) cylinder(d = display_hole_d, h = display_size_bottom.z + .02, $fn = 20);
    }

    translate([(display_size_board.x - display_size_bottom.x) / 2, (display_size_board.y - display_size_bottom.y) / 2])
        color("black") cube(display_size_bottom);

    translate([0, 0, display_size_bottom.z]) {
        difference() {
            color("green") cube(display_size_board);

            echo("Left", (display_size_board.y - display_hole_pos[0].y) - display_hole_pos[1].y);
            echo("Right", (display_size_board.y - display_hole_pos[2].y) - display_hole_pos[3].y);
            echo("Top", (display_size_board.x - display_hole_pos[2].x) - display_hole_pos[0].x);
            echo("Bottom", (display_size_board.x - display_hole_pos[3].x) - display_hole_pos[1].x);

            /* top left     */ Hole(display_hole_pos[0].x, display_size_board.y - display_hole_pos[0].y);
            /* bottom left  */ Hole(display_hole_pos[1].x, display_hole_pos[1].y);
            /* top right    */ Hole(display_size_board.x - display_hole_pos[2].x, display_size_board.y - display_hole_pos[2].y);
            /* bottom right */ Hole(display_size_board.x - display_hole_pos[3].x, display_hole_pos[3].y);
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
    union() {
        for (i = [0:1:batts - 1]) {
            translate([i * batt_holder_size.x - i, 0]) {
                difference() {
                    cube(batt_holder_size);
                    translate([1, 1, 1]) cube([battery_dia, 66, 100]);

                    // debug:
                    // translate([-.01,-.01,-.01]) cube([1 + .02, batt_holder_size.y + .02, 100]);
                }
                if (DEBUG) translate([battery_dia / 2 + 1, 0 + 1, battery_dia / 2 + 1]) color("grey") union() {
                    rotate([- 90]) cylinder(d = battery_dia, h = battery_length, $fn = 25);

                    // contacts
                    translate([- 5, 0]) cube([10, 1, 15]);
                    translate([- 5, battery_length - 1]) cube([10, 1, 15]);
                }

            }
        }
    }
}

module SdSlot() {
    union() {
        color("black")  cube(sd_size);
        color("black") translate([sd_hold_x, sd_size.y - .01]) cube(sd_hold_size);
        color("black") translate([sd_hold_x, - sd_hold_size.y + .01]) cube(sd_hold_size);
        color("gray") translate([- .1, 1.5, .8]) cube([.1, sd_size.y - 3, sd_size.z - .8]);
    }
}

module DispSupport(x, y, stands = true) {
    translate([x, y]) union() {
        outter_dia = display_hole_d + 6;
        difference() {
            h = display_z + display_size_bottom.z - .01;
            color("red") cylinder(d = outter_dia, h = h, $fn = 20);
            color("yellow") translate([0, 0, .1]) cylinder(d = display_hole_d, h = 100, $fn = 20);
        }


        color("green") if (stands) {
            l = 6;
            h = 3;

            translate([- l - outter_dia / 2, - 1]) cube([l + 1, 2, h]);
            translate([outter_dia / 2 - 1, - 1]) cube([l + 1, 2, h]);
            translate([- 1, - l - outter_dia / 2]) cube([2, l + 1, h]);
            translate([- 1, - 1 + outter_dia / 2]) cube([2, l + 1, h]);}
    }
}

module DispSuppStand(x, y) {
    outter_dia = display_hole_d + 6;

    translate([x, y]) union() {
        color("lightgreen") {
            l = 6;
            h = 3;

            // left
            translate([- l - outter_dia / 2, - 3 - inset / 2]) cube([l, 2, h]);
            translate([- l - outter_dia / 2, 1 + inset / 2]) cube([l, 2, h]);

            // right
            translate([outter_dia / 2, - 3 - inset / 2]) cube([l, 2, h]);
            translate([outter_dia / 2, 1 + inset / 2]) cube([l, 2, h]);

            // up
            translate([- 3 - inset, - l - outter_dia / 2]) cube([2, l, h]);
            translate([1 + inset, - l - outter_dia / 2]) cube([2, l, h]);

            // down
            translate([- 3 - inset, outter_dia / 2]) cube([2, l, h]);
            translate([1 + inset, outter_dia / 2]) cube([2, l, h]);
        }
    }
}

// -------------------------------------

// main
module Main() {
    union() {
        difference() {
            #cube(box);
            translate([fatness, fatness, - .01]) color("blue") cube([box_inner.x, box_inner.y, 100]);

            // DEBUG
            // translate([fatness, - .01, fatness]) cube([box_inner.x, fatness + .02, 100]); // front wall
            // translate([fatness, box_y + fatness - .01, fatness]) cube([box_inner.x, fatness + .02, 100]); // back wall
            // translate([box.x - fatness - .01, - .01, fatness]) cube([fatness + .02, box.y + .02, 100]); // right wall

            translate([fatness, fatness]) {
                // charger hole
                translate([- fatness - .01, charger_y + 3.9, .2]) {
                    cube([fatness + .02, 11, 5]);
                }

                // switch
                translate([- fatness - .01, switch_y + 11.2, switch_z]) {
                    rotate([0, 90])cylinder(d = switch_head_dia() + 1, h = fatness + .02, $fn = 30);
                }

                // SD slot
                translate([- fatness - .01, sdslot_y + 2.65, .8]) {
                    cube([fatness + .02, sd_size.y - 2.95, sd_size.z - .8]);
                }
            }
        }

        translate([fatness, fatness, fatness - .01]) {
            translate(disp_pos) {
                *if (DEBUG) translate([0, 0, display_z]) Display();


                /* bottom left  */ DispSupport(display_hole_pos[1].x, display_hole_pos[1].y);
                /* top right    */ DispSupport(display_size_board.x - display_hole_pos[2].x, display_size_board.y - display_hole_pos[2].y);
                /* bottom right */ DispSupport(display_size_board.x - display_hole_pos[3].x, display_hole_pos[3].y);
            }


            // MFRC
            translate([70.5, 1.4, - .01]) union() {
                f = 1.5;

                #if (DEBUG) translate([f + inset / 2, MFRC_board_size().y + f + inset / 2, box_inner.z + .01]) rotate([180]) MFRC_board();

                difference() {
                    outter = [MFRC_board_size().x + inset + 2 * f, MFRC_board_size().y + inset + 2 * f, box_inner.z];
                    cube(outter);
                    translate([f, f]) cube([MFRC_board_size().x + inset, MFRC_board_size().y + inset, 100]);

                    translate([- .01, 18, 22]) cube([2, 15, 100]);
                    translate([- .01, 4]) cube([100, outter.y - 2 * 4, 18]);
                    translate([10, - .01]) cube([outter.x - 20, 100, 25]);

                    // debug MRFC stand:
                    // translate([1, -1]) cube([MFRC_board_size().x + 2 * inset, 100, 100]);
                }

                //supports
                translate([1, 1]) {
                    translate([18, 5]) difference() {
                        outter = [MFRC_board_size().x - 2 * 18, MFRC_board_size().y - 2 * 5, box_inner.z - MFRC_board_size().z - inset];
                        cube(outter);
                        translate([f, f]) cube([outter.x - 2 * f, outter.y - 2 * f, 100]);

                        translate([- .01, 4]) cube([100, outter.y - 2 * 4, 25]);
                        translate([4, - .01]) cube([outter.x - 2 * 4, 100, 25]);
                    }
                }

                // connections
                translate([19, f - .01, 25])  cube([f, MFRC_board_size().y + inset + .02, 2]);
                translate([41.4, f - .01, 25])  cube([f, MFRC_board_size().y + inset + .02, 2]);
            }

            translate(batt_holder_pos) BatteryHolder();
        }
    }
}

module Cover() {
    height = 15;

    difference() {
        # union() {
            cube([box.x, box.y, fatness]);
            translate([fatness + inset, fatness + inset, - height + .01]) color("red") difference() {
                size = [box_inner.x - 2 * inset, box_inner.y - 2 * inset, height + .02];
                cube(size);
                translate([1, 1, - .02]) cube([size.x - 2, size.y - 2, 100]);
            }
        }

        // display hole
        d_hole_pos = [disp_pos.x + (display_size_board.x - display_size_top.x) / 2 + fatness - inset,
                        disp_pos.y + (display_size_board.y - display_size_top.y) / 2 + fatness - inset, - .02];

        translate(d_hole_pos) {
            cube([display_size_top.x + 2 * inset, display_size_top.y + 2 * inset, 100]);
        }

        // tag hole
        translate([box.x / 2, 23, fatness - 1]) cylinder(d = 40, h = 100, $fn = 50);

        // DEBUG
        // translate([-.01, -.01, -.01]) cube([box.x + .02, box.y + .02, fatness + .02]); // top
        // translate([fatness, fatness - .01, - height]) cube([box.x - 2 * fatness, fatness, height]); // front
        // translate([fatness, box.y - 2 * fatness - .01, - height]) cube([box.x - 2 * fatness, fatness, height]); // back
        // translate([box.x - 2 * fatness, fatness - .01, - height]) cube([fatness + .02, box.y - 2 * fatness, height]); // right
        // translate([fatness - .01, fatness - .01, - height]) cube([fatness + .02, box.y - 2 * fatness, height]); // left
    }
}

module Bottom() {
    height = 15;

    difference() {
        union() {
            # color("violet") cube([box.x, box.y, fatness]);

            translate([fatness + inset, fatness + inset, - .01]) color("darkred") difference() {
                size = [box_inner.x - 2 * inset, box_inner.y - 2 * inset, height + .02];
                cube(size);
                translate([1, 1, - .02]) cube([size.x - 2, size.y - 2, 100]);

                // charger hole
                translate([- .01, charger_y, 2.8]) {
                    cube([fatness + .02, charger_size.x + 1.5, 5]);
                }

                // switch
                translate([- 0 - .01, switch_y + 1.3, switch_z - 6]) {
                    color("blue") cube([1 + .02, 19, 20]);
                }

                // SD slot
                translate([- 0 - .01, sdslot_y + 2.4, 3]) {
                    cube([fatness + .02, sd_size.y - 3, sd_size.z - 1]);
                }
            }

            translate([fatness + 1.2, fatness + 1.2, fatness - .01]) {
                // switch
                translate([- 1 - .01, switch_y, - .01]) {
                    size = [5.5, 20, switch_z + 10];
                    difference() {
                        cube(size);
                        color("red") translate([- .01, fatness]) cube([10, size.y - 2 * fatness, size.z - 1]);
                    }
                    #translate([size.x - .01, 0]) difference() {
                        color("green") cube([fatness, size.y, size.z]);
                        // the positioning here is... wild :-|
                        translate([- .01, size.y / 2, switch_z + .3]) rotate([0, 90])
                            cylinder(d = switch_hole_dia(), h = fatness + .02, $fn = 30);
                    }

                    if (DEBUG) translate([23, size.y / 2, switch_z + .3]) rotate([0, - 90]) Switch(fatness + inset);
                }

                // charger
                translate([- 1 - .01, charger_y]) {
                    translate([- .01, - 1 - inset]) {
                        size = [5, charger_size.x + 2 * (1 + inset), 6];
                        difference() {
                            cube(size);
                            translate([- .01, 1]) cube([100, charger_size.x + 2 * inset, 4.8]);
                        }
                    }

                    translate([charger_size.z - 5, - 1 - inset]) difference() {
                        cube([5 + 1 + inset, charger_size.x + 2 * 1 + inset, 3]);
                        translate([- .01, 1]) cube([5 + inset, charger_size.x + inset, 100]);
                    }

                    if (DEBUG)  translate([28, - inset / 2, 1]) rotate([90, 180, - 90]) Charger();
                }

                // SD slot
                translate([inset / 2, sdslot_y]) union() {
                    translate([- inset / 2, - 1 - inset]) difference() {
                        size = [sd_size.x + 1 + inset, sd_size.y + 2 * (1 + inset), 5];
                        cube(size);
                        translate([- .01, 1]) cube([sd_size.x + inset, sd_size.y + 2 * inset, 100]);
                        translate([0, 4]) cube([100, size.y - 8, 100]);

                        translate([sd_hold_x - inset / 2, - .01]) cube([sd_hold_size.x + 2 * inset, 100, 100]);
                    }

                    if (DEBUG) SdSlot();
                }

                // display supports / stands
                translate(disp_pos) translate([- 1 - inset, - 1 - inset]) {

                    /* top left    */
                    union() {
                        difference() {
                            DispSupport(display_hole_pos[0].x, display_size_board.y - display_hole_pos[0].y, false);
                            translate([display_hole_pos[0].x - 10, display_size_board.y - display_hole_pos[0].y - 10])
                                cube([20, 20, 5 - .01]);
                        }

                        translate([display_hole_pos[0].x - 7.2, display_size_board.y - display_hole_pos[0].y - 4.499]) difference() {
                            color("orange") cube([11.73, 9, 5]);
                            translate([2.1 - inset / 2, - .01]) color("blue") cube([sd_hold_size.x + inset, sd_hold_size.y, 4.3]);
                        }
                    }

                    /* bottom left */ DispSuppStand(display_hole_pos[1].x, display_hole_pos[1].y);
                    /* top right   */ DispSuppStand(display_size_board.x - display_hole_pos[2].x, display_size_board.y - display_hole_pos[2]
                    .y);
                    /* bottom right*/ DispSuppStand(display_size_board.x - display_hole_pos[3].x, display_hole_pos[3].y);
                }

                if (DEBUG) translate([inset, inset, 2.2]) Pcb();

                // board bed
                translate([- 1, - 1]) difference() {
                    cube([pcb_size.x + 2 * (1 + inset), pcb_size.y + 2 * (1 + inset), 10]);
                    translate([1, 1, - .01]) cube([pcb_size.x + 2 * inset, pcb_size.y + 2 * inset, 100]);

                    // saving space
                    translate([5, - .01]) cube([52, 1 + .02, 100]);
                    translate([pcb_size.x + 1, 9]) cube([100, rtc_size.y + 1, 100]);
                }

                // MFRC border
                translate([68.3 - inset, - 1, - .01]) difference() {
                    inner_f = 1.5;
                    outter_f = 1;
                    inner = [MFRC_board_size().x + inset + 2 * inner_f, MFRC_board_size().y + inset + 2 * inner_f, box_inner.z];
                    outter = [inner.x + 2 * outter_f + 2 * inset, inner.y + 2 * outter_f + 2 * inset, 10];
                    cube(outter);
                    translate([outter_f, outter_f, - .01]) cube([inner.x + 2 * inset, inner.y + 2 * inset, 100]);

                    translate([- .01, 18, 22]) cube([2, 15, 100]);
                    translate([- .01, 4]) cube([100, outter.y - 2 * 4, 18]);
                    translate([10, - .01]) cube([outter.x - 20, 100, 25]);
                }
            }

            // battery holder border
            translate([fatness + batt_holder_pos.x - 1 - inset, fatness + batt_holder_pos.y - 1 - inset, fatness - .01]) difference() {
                outter_f = 1;

                inner = [batt_holder_size.x * batts - (batts - 1), batt_holder_size.y, batt_holder_size.z];
                outter = [inner.x + 2 * outter_f + 2 * inset, inner.y + 2 * outter_f + 2 * inset, 5];

                cube(outter);
                translate([outter_f, outter_f, - .01]) cube([inner.x + 2 * inset, inner.y + 2 * inset, 100]);

                translate([- .01, 18, 22]) cube([2, 15, 100]);
                translate([- .01, 4]) cube([200, outter.y - 2 * 4, 18]);
                translate([10, - .01]) cube([outter.x - 20, 200, 25]);
            }
        }

        // DEBUG
        // translate([- .01, - .01, - .01]) cube([box.x + .02, box.y + .02, fatness + .02]); // top
        // translate([fatness, fatness - .01, - height]) cube([box.x - 2 * fatness, fatness, height]); // front
        // translate([fatness, box.y - 2 * fatness - .01, - height]) cube([box.x - 2 * fatness, fatness, height]); // back
        // translate([box.x - 2 * fatness, fatness - .01, - height]) cube([fatness + .02, box.y - 2 * fatness, height]); // right
        // translate([fatness + inset - .01, fatness - .01, - 0.1]) cube([1 + .02, box.y - 2 * fatness, 100]); // left
    }
}

translate([0, 0, fatness + inset / 2])
    difference() {
        Main();


        //    /* PRINTING SETTINGS: */
        //
        //    // left wall:
        //
        //    // PCB
        //    translate([- .01, - .01, 10]) cube([fatness + .02, 55, 100]);
        //    // switch, charger
        //    translate([- .01, switch_y - 5, 20]) cube([fatness + .02, 100, 100]);
        //    translate([- .01, charger_y + 3, 10]) cube([fatness + .02, 100, 100]);
        //
        //
        //    // back wall
        //    translate([- .01, box.y - fatness - .01, fatness]) cube([box.x + .02, fatness + .02, 100]);
        //
        //    // front wall
        //    translate([- .01, -.01, 10]) cube([box.x + .02, fatness + .02, 100]);
        //
        //
        //    // right wall
        //    translate([box.x - fatness - .01, - .01, fatness]) cube([fatness + .02, box.y + .02, 100]);
    }

//translate([0, 0]) // for modelling
    translate([250, 0]) // for print
    Bottom();

//translate([0, 0, fatness + box.z + inset]) // for modelling
    translate([0, - 10]) rotate([180]) // for print
    Cover();

