VENEER_WIDTH = 12;
CONTAINER_HEIGHT = 32;
CONTAINER_WIDTH = 146;
BUTTON_R = 8;
BUTTON_X1 = VENEER_WIDTH + BUTTON_R + 0;
BUTTON_Y = VENEER_WIDTH + 0;
BUTTON_DIST = 20;
POWER_R = 5;
EPSILON = 0.01;

// bottom
cube([CONTAINER_WIDTH + 2 * VENEER_WIDTH, CONTAINER_HEIGHT + VENEER_WIDTH, VENEER_WIDTH]);
echo("Bottom/top: ", CONTAINER_WIDTH + 2 * VENEER_WIDTH, "x", CONTAINER_HEIGHT + VENEER_WIDTH);

// left
translate([0, 0, VENEER_WIDTH]) {
    cube([VENEER_WIDTH, CONTAINER_HEIGHT + VENEER_WIDTH, CONTAINER_HEIGHT]);
}
echo("Left/right: ", CONTAINER_HEIGHT + VENEER_WIDTH, "x", CONTAINER_HEIGHT);
    
// right    
translate([CONTAINER_WIDTH+VENEER_WIDTH, 0, VENEER_WIDTH]) {    
    cube([VENEER_WIDTH, CONTAINER_HEIGHT + VENEER_WIDTH, CONTAINER_HEIGHT]);
}

// top
translate([0, 0, CONTAINER_HEIGHT + VENEER_WIDTH]) {
    difference() {
        // top
        cube([CONTAINER_WIDTH + 2 * VENEER_WIDTH, CONTAINER_HEIGHT + VENEER_WIDTH, VENEER_WIDTH]);
        
        // button holes
        for (i = [0:2]) {
            translate([BUTTON_X1 + i * BUTTON_DIST, BUTTON_R + BUTTON_Y, -EPSILON]) {
                cylinder(h=VENEER_WIDTH + 2*EPSILON, r=BUTTON_R);
            }
        }
    }
    
    // buttons
    for (i = [0:2]) {
        translate([BUTTON_X1 + i * BUTTON_DIST, BUTTON_Y + BUTTON_R, 1]) {
            color("#ffff99") {
                cylinder(h=VENEER_WIDTH + 2*EPSILON, r=BUTTON_R - EPSILON);
            }
        }
    }
}

// backside
translate([0, CONTAINER_HEIGHT + VENEER_WIDTH, 0]) {
    difference() {
        cube([CONTAINER_WIDTH + 2 * VENEER_WIDTH, VENEER_WIDTH, CONTAINER_HEIGHT + 2 * VENEER_WIDTH]);
        translate([CONTAINER_WIDTH + VENEER_WIDTH - POWER_R, -EPSILON, VENEER_WIDTH + POWER_R]) {
            rotate([270, 0, 0]) {
                cylinder(h=VENEER_WIDTH + 2 * EPSILON, r=POWER_R);
            }
        }
        translate([VENEER_WIDTH/2, 0, VENEER_WIDTH/2]) {
            rotate([270, 0, 0]) {
                cylinder(h=VENEER_WIDTH + 2 * EPSILON, r=1);
            }
        }
        translate([CONTAINER_WIDTH+VENEER_WIDTH*1.5, 0, VENEER_WIDTH/2]) {
            rotate([270, 0, 0]) {
                cylinder(h=VENEER_WIDTH + 2 * EPSILON, r=1);
            }
        }
        translate([CONTAINER_WIDTH+VENEER_WIDTH*1.5, 0, CONTAINER_HEIGHT + VENEER_WIDTH * 1.5]) {
            rotate([270, 0, 0]) {
                cylinder(h=VENEER_WIDTH + 2 * EPSILON, r=1);
            }
        }
        translate([VENEER_WIDTH/2, 0, CONTAINER_HEIGHT + VENEER_WIDTH * 1.5]) {
            rotate([270, 0, 0]) {
                cylinder(h=VENEER_WIDTH + 2 * EPSILON, r=1);
            }
        }
    }
}
echo("Back: ", CONTAINER_WIDTH + 2 * VENEER_WIDTH, "x", CONTAINER_HEIGHT + 2 * VENEER_WIDTH);


// front film
translate([0, -EPSILON, 0]) {
    color("#000000", 0.4) {
        cube([CONTAINER_WIDTH + 2 * VENEER_WIDTH, EPSILON, CONTAINER_HEIGHT + 2 * VENEER_WIDTH]);
    }
}