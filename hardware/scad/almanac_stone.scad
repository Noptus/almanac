// ===========================================================================
// Almanac Stone -- prototype enclosure ("Toblerone" triangular prism)
// Parametric OpenSCAD. Units: millimetres.
//
// Built around the Waveshare 10.85" e-Paper HAT+ (G):
//   panel outline  270.56 x 105.92 x 1.20 mm ; active 259.76 x 91.68 mm
//   SPI 4-colour (R/Y/B/W) ; driver HAT 65.0 x 30.5 mm
//
// Cross-section is a right-angled triangle in Y-Z, extruded along X (width).
//   - vertical BACK wall (at y=0)
//   - horizontal BASE  (at z=0)
//   - sloped FRONT face carrying the screen, tilted `front_tilt` from vertical
// The screen lies on the sloped front face; a crystal well sits front-left.
//
// part = "assembly" | "left" | "right" | "base" | "bezel"
// The body is ~300 mm wide so it prints in LEFT + RIGHT halves (seam at X mid),
// with a screwed-on BASE plate for electronics access and a snap BEZEL.
// ===========================================================================

/* [Render] */
part = "assembly"; // [assembly, left, right, base, bezel]

/* [Panel: Waveshare 10.85 (G)] */
panel_w   = 270.56;   // outline width  (X on the face)
panel_h   = 105.92;   // outline height (up the slope)
panel_t   = 1.20;     // glass thickness
active_w  = 259.76;
active_h  = 91.68;

/* [Body] */
body_width = 300;     // X length of the prism
depth      = 150;     // Y footprint (base)
height     = 120;     // Z height of the back wall
front_tilt = 20;      // front face lean from vertical (deg); 0 = vertical screen
wall       = 3.2;

/* [Screen window] */
bezel_overlap = 4;    // bezel covers this much of active edge
face_up_frac  = 0.50; // centre of screen along the slope (0 base .. 1 apex)

/* [Crystal well] */
// The screen fills most of the face width, so the crystal well sits LOW on the
// front face, below the screen window, front-left (as in the product render).
crystal_w    = 26;
crystal_h    = 40;
crystal_deep = 18;
crystal_from_left = 46;   // from the left edge of the face to the well centre
crystal_up_frac   = 0.12; // low on the slope, clear of the screen
led_hole     = 6;

/* [Assembly] */
dowel_d = 4.2;
dowels  = 3;
screw_d = 3.4;
tol     = 0.30;

/* [Quality] */
$fn = 48;
eps = 0.02;

// ---------------------------------------------------------------------------
// Cross-section triangle (Y-Z). Back wall vertical at y=0.
//   A = (0,0)         base-back corner
//   B = (depth,0)     base-front corner
//   C = (0,height)    top of back wall
// The FRONT face is the hypotenuse B->C. Its lean from vertical:
//   we tilt by shifting C forward by height*tan(front_tilt).
// ---------------------------------------------------------------------------
Cx = height * tan(front_tilt);
tri = [[0,0],[depth,0],[Cx,height]];

// Front face geometry (segment B=(depth,0) -> C=(Cx,height))
face_dy = Cx - depth;
face_dz = height;
face_len = sqrt(face_dy*face_dy + face_dz*face_dz);
face_ang = atan2(face_dz, face_dy);   // angle of face direction from +Y axis

module solid_prism(w) {
    rotate([90,0,90]) linear_extrude(height=w) polygon(tri);
}

// Inner cavity: same triangle scaled down by wall on each side (simple offset).
module inner_prism(w) {
    // offset() shrinks the 2D triangle uniformly by `wall`.
    translate([wall, 0, 0])
        rotate([90,0,90]) linear_extrude(height = w - 2*wall)
            offset(delta = -wall) polygon(tri);
}

// Place children onto the FRONT face.
//   u = distance along the slope from corner B (0..face_len)
//   x = position along width from body centre
//   into = depth into the body (negative goes inward)
// After the transform, child's +Z points OUT of the face (normal), local X is
// width, local Y runs up the slope.
module on_face(u, x=0, into=0) {
    // Point on the face in Y-Z:
    py = depth + (face_dy/face_len)*u;
    pz = (face_dz/face_len)*u;
    translate([body_width/2 + x, py, pz])
        // rotate local +Z to the outward face normal.
        // Face normal points up-and-forward: normal angle = face_ang - 90 in Y-Z.
        rotate([face_ang, 0, 0])       // tilt so local Y follows the slope
            rotate([0,0,0])
                translate([0,0,into])
                    children();
}

// ---------------------------------------------------------------------------
// Cutouts
// ---------------------------------------------------------------------------
module screen_cut() {
    u = face_len * face_up_frac;
    win_w = active_w - 2*bezel_overlap;
    win_h = active_h - 2*bezel_overlap;
    // through window
    on_face(u, 0, -50) cube([win_w, win_h, 100], center=true);
    // panel glass rebate (from the outside)
    on_face(u, 0, -panel_t) cube([panel_w+2*tol, panel_h+2*tol, panel_t+2], center=true);
    // FPC slot at the lower edge of the panel, into the body
    on_face(u - panel_h/2 - 2, 0, -40) cube([46, 16, 80], center=true);
}

module crystal_cut() {
    u = face_len * crystal_up_frac;
    xoff = -(body_width/2) + crystal_from_left;
    on_face(u, xoff, -crystal_deep) {
        // rounded well
        minkowski() { cube([crystal_w-6, crystal_h-6, crystal_deep], center=true); cylinder(r=3,h=eps); }
    }
    // light hole from well to interior
    on_face(u, xoff, -crystal_deep-wall) cylinder(d=led_hole, h=crystal_deep+wall+6);
}

module dowel_holes() {
    for (i=[0:dowels-1]) {
        yy = depth*(i+1)/(dowels+1);
        // hole runs along X through the seam plane
        translate([body_width/2, yy, height*0.18])
            rotate([0,90,0]) cylinder(d=dowel_d, h=60, center=true);
    }
}

module base_pocket() {
    // rebate for the screwed-on base plate
    translate([wall+1, wall+1, -eps])
        cube([body_width-2*wall-2, depth-2*wall-2, 3+eps]);
}

// ---------------------------------------------------------------------------
// Body
// ---------------------------------------------------------------------------
module body() {
    difference() {
        solid_prism(body_width);
        inner_prism(body_width);
        screen_cut();
        crystal_cut();
        base_pocket();
        dowel_holes();
    }
}

module left_half()  { intersection() { body(); translate([-1,-10,-10]) cube([body_width/2+1, depth+20, height+20]); } }
module right_half() { intersection() { body(); translate([body_width/2,-10,-10]) cube([body_width/2+1, depth+20, height+20]); } }

module base_plate() {
    bw = depth-2*wall-2;  bl = body_width-2*wall-2;
    difference() {
        cube([bl, bw, 3]);
        for (x=[10, bl-10], y=[10, bw-10]) translate([x,y,-eps]) cylinder(d=screw_d, h=6);
        for (i=[0:5]) translate([40+i*35, bw/2-15, -eps]) cube([6,30,6]); // vents
    }
}

module bezel() {
    win_w = active_w - 2*bezel_overlap;
    win_h = active_h - 2*bezel_overlap;
    outer_w = panel_w + 12;  outer_h = panel_h + 12;
    difference() {
        cube([outer_w, outer_h, 3.6], center=true);
        translate([0,0,-1])   cube([win_w, win_h, 8], center=true);              // opening
        translate([0,0,1.7])  cube([panel_w+2*tol, panel_h+2*tol, 3], center=true); // glass rebate
    }
}

// ---------------------------------------------------------------------------
// Selector
// ---------------------------------------------------------------------------
if      (part=="assembly") {
    color([0.17,0.16,0.20]) body();
    u = face_len*face_up_frac;
    color([0.05,0.05,0.05,0.8]) on_face(u, 0, -0.4) cube([active_w, active_h, 1.2], center=true);
}
else if (part=="left")  left_half();
else if (part=="right") right_half();
else if (part=="base")  base_plate();
else if (part=="bezel") bezel();
