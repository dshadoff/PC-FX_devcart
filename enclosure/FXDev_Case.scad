//
// PC-FX dev cartridge shell
//
// (c) 2024,  David Shadoff
//

// definitions of what to show
// Please adjust these
//
// In order to create a full shell, you will need to run STL once
// with 'show_half = 0', and once with 'show_half = 1'
//
show_half = 1;      // 0 = bottom;     1 = top
show_board = 0;     // 0 = don't show; 1 = show


$fn=30;

// case outer dimensions
case_len = 97;
case_width = 54;
case_thick = 13.5;
mink_edge = 1;

// case card-edge dimensions
end_hole_deep = 8.5;
end_edge_thick = 1.5;

// PC board dimensions
board_width = 50;
board_edge_tol = 0.3;
tray_width = board_width+(2*board_edge_tol);
board_thick = 1.6;
board_thick_tol = 0.2;
usb_edge = 1.5;
overshoot = 0.1;

//support cylinders
cyl_outer_rad = (4.0/2);  // 4.0 mm (support hole in PC board is actually 4.4mm
cyl_inner_rad = (2.2/2);  // 2.0 mm (for ?? screw shaft)
screwhead_depth = 2.0;
screwhead_rad = (3.6/2);
nuthead_depth = 1.5;
nuthead_rad = (4.6/2);

// relative to board
bottom_z = 11.3654;
top_z = 83.509;
left_x = 46.4086;
right_x = 3.5814;

//Text
text_size = 9;
text_size_2 = 8;
text_depth = 0.5;

// bottom_well
bottom_well_depth = 3.5;

// top square
hollow_1_width = 44;
hollow_1_thick = 3.5;
hollow_1_bottom = 16;
hollow_1_len = 65;

//pinhole
pinhole_radius = (1.4/2);

// grip
grip_length = 32;



module support_cylinder(x_pos, y_pos) {     // additive, meant for bottom side
    translate([((case_width - board_width)/2)+x_pos, (case_thick/2), (usb_edge+y_pos)]) {
      rotate([90, 0,0])
      cylinder(h=(case_thick/2), r1=cyl_outer_rad, r2=cyl_outer_rad);
    }
}

module screw_shaft(x_pos, y_pos) {          // subtractive, applies to both sides
    translate([((case_width - board_width)/2)+x_pos, (case_thick+overshoot), (usb_edge+y_pos)]) {
      rotate([90, 0,0])
      cylinder(h=(case_thick+(overshoot*2)), r1=cyl_inner_rad, r2=cyl_inner_rad);
    }
}

module screwhead_sink(x_pos, y_pos) {       // subtractive, applies to bottom side
    translate([((case_width - board_width)/2)+x_pos, screwhead_depth, (usb_edge+y_pos)]) {
      rotate([90, 0,0])
      cylinder(h=screwhead_depth+overshoot, r1=screwhead_rad, r2=screwhead_rad);
    }
}

module nuthead_sink(x_pos, y_pos) {         // subtractive, applies to top side
    translate([((case_width - board_width)/2)+x_pos, (case_thick+overshoot), (usb_edge+y_pos)]) {
      rotate([90, 0,0])
      //TODO: make this into hexagonal well
      cylinder(h=nuthead_depth+overshoot, r1=nuthead_rad, r2=nuthead_rad, $fn=6);
    }
}

module reset_pinhole(x_pos, y_pos) {
    translate([x_pos,(case_thick/2)-overshoot, y_pos])
        rotate([-90,0,0])
            cylinder(h = (case_thick/2)+(overshoot*2), r = pinhole_radius);
}

module grip_inset(length, iterations) {
    translate([length,0,0])
    rotate([0,-90,0])
    linear_extrude(length) {
        translate([0,-0.9])
        rotate(-45)
        union() {
            for(i= [0:1:iterations] ) {
                translate([i,i])
                square([2,2]);
            }
        }
    }
}

// This is the main "program" for the shell
//
difference() {
    union() {
        difference() {

        // overall outer shape
            translate([mink_edge,mink_edge,mink_edge])
            minkowski()
            {
               cube([(case_width-(2*mink_edge)),(case_thick-(2*mink_edge)),(case_len-(2*mink_edge))]);
               sphere(r=mink_edge,$fn=50);
            }

            // hollow-out at card-edge
            translate([(end_edge_thick), (end_edge_thick), (case_len-end_hole_deep)])
                cube( [ (case_width-(2*end_edge_thick)),(case_thick-(2*end_edge_thick)), end_hole_deep+end_edge_thick ] );

            // if bottom half, cut off top half
            if (show_half == 0) {
                translate([-overshoot, (case_thick/2), -overshoot]) {
                    cube([(case_width+(2*overshoot)), ((case_thick/2)+(2*overshoot)), (case_len+(2*overshoot))]);
                }

                // cut out tray for PC board
                translate([(case_width - tray_width)/2, (case_thick/2)-(board_thick+board_thick_tol), (usb_edge-board_edge_tol)]) {
                    cube([tray_width,(board_thick+(2*board_thick_tol)),case_len]);
                }

                // cut out additional unnecessary well below PC board
                translate([((case_width - 38)/2)+(38/2), (case_thick/2)-board_thick-bottom_well_depth+(2*overshoot),(60/2)+18]) {
                    rotate([-90, 0,0]) {
                        linear_extrude(bottom_well_depth+overshoot) {
                            offset(r = -5) {
                                minkowski() {
                                    union() {
                                        square([30,50], center=true);
                                        translate([-10,-28])
                                        square([20,56]);
                                    }
                                    circle(10);
                                }
                            }
                        }
                    }
                }
            }
            else {
                // if top half, cut off bottom
                translate([-overshoot, -overshoot, -overshoot]) {
                    cube([(case_width+(2*overshoot)), ((case_thick/2)+(overshoot)), (case_len+(2*overshoot))]);
                }

                // Subtract bubble for electronic components (top half only)
                translate([((case_width - 38)/2)+(38/2), (case_thick/2)-overshoot,(60/2)+18]) {
                    rotate([-90, 0,0]) {
                        linear_extrude(4.5+overshoot) {
                            offset(r = -5) {
                                minkowski() {
                                    union() {
                                        square([34,50], center=true);
                                        translate([-10,-28])
                                        square([20,60]);
                                    }
                                    circle(10);
                                }
                            }
                        }
                    }
                }

                // Subtract LED well (top half only)
                translate([(case_width - 18), (case_thick/2)-overshoot, 10]) {
                    rotate([-90, 0,0]) {
                        linear_extrude(1.5) {
                            minkowski() {
                               polygon([[0,-2],[3,-3],[7,0],[7,5],[0,5]]);
                                circle(2);
                            }
                        }
                    }
                }   
            }
        }
 
        // add support cylinders (bottom only)
        if (show_half == 0) {
            support_cylinder(left_x, bottom_z);     // bottom left
            support_cylinder(right_x, bottom_z);    // bottom right
            support_cylinder(left_x, top_z);        // top left
            support_cylinder(right_x, top_z);       // top right
        }
    }
    
    // subtract support cylinder screw shafts (both halves)
    screw_shaft(left_x, bottom_z);                  // bottom left
    screw_shaft(right_x, bottom_z);                 // bottom right
    screw_shaft(left_x, top_z);                     // top left
    screw_shaft(right_x, top_z);                    // top right

    // subtract support screwhead wells (bottom only)
    if (show_half == 0) {
        screwhead_sink(left_x, bottom_z);           // bottom left
        screwhead_sink(right_x, bottom_z);          // bottom right
        screwhead_sink(left_x, top_z);              // top left
        screwhead_sink(right_x, top_z);             // top right
    }
    else {
    // subtract support nuts hexagon wells (top only)
        nuthead_sink(left_x, bottom_z);             // bottom left
        nuthead_sink(right_x, bottom_z);            // bottom right
        nuthead_sink(left_x, top_z);                // top left
        nuthead_sink(right_x, top_z);               // top right
    }

    // Subtract reset button pinholes (top half only)
    if (show_half == 1) {
        reset_pinhole(9.75, 41);
        reset_pinhole(9.75, 31);
    }


    // Subtract USB hole (both halves)
    translate([23.5,(case_thick/2)+0.5, -overshoot]) {
        linear_extrude(12) {
            minkowski() {            
                square([7.5,2]);
                circle(1);
            }
        }
    }

    // Subtract grip ridges - both halves, but different position for each
    if (show_half == 1) {
        translate([((case_width-grip_length)/2), case_thick, 1.5]) {
            mirror([0,1,0]) {
                grip_inset(grip_length, 11);

            }
        }
    }
    else {
        translate([((case_width-grip_length)/2), 0, 1.5]) {
            grip_inset(grip_length, 11);
        }
    }

    // Subtract Text (bottom half)
    if (show_half == 0) {
        translate([((case_width/2)+(text_size/2)), text_depth, 72])
        rotate([0,90,0])
        rotate([90,0,0])
        linear_extrude(text_depth+overshoot)
        text("PC-FX", size = text_size, valign="baseline", font="Liberation Sans:style=Bold", language="en", script="latin");

        translate([((case_width/2)-(1.2*text_size_2)), text_depth, 78])
        rotate([0,90,0])
        rotate([90,0,0])
        linear_extrude(text_depth+overshoot)
        text("DEVCART", size = text_size_2, valign="baseline", font="Liberation Sans:style=Bold", language="en", script="latin");
    }    

    // Subtract Text (top half)
    if (show_half == 1) {
        translate([((case_width-text_size)/2), case_thick-text_depth+overshoot, 30])
        rotate([0,-90,0])
        rotate([-90,0,0])
        linear_extrude(text_depth+overshoot)
        text("FX-DEV", size = text_size, valign="baseline", font="Liberation Sans:style=Bold", language="en", script="latin");
    }

}

// Test together with sample board for fit

if (show_board == 1) {
    color("grey",1)
        translate([0,(case_thick/2)-board_thick-(board_thick_tol/2),usb_edge])
            rotate([90,0,0])
                rotate([0,180,0])
                    translate([(-170.2-((case_width-board_width)/2)),(147.4),0])
                        import("Devboard.stl");
}




