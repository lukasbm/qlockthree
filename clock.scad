ledSpacing = 33.3; // Spacing between LEDs
cols = 11;
rows = 10;
sideLength = 380; // Length of the side of the square plate non-front plate layers.
assert(ledSpacing * cols <= sideLength);


module FrontPlate(fontSize = 20, sideLength = 450, spacing = 10, margin = 50) {
    plateThickness = 2; // Thickness of the glass plate

    // Create the base plate
    difference() {
        cube([sideLength, sideLength, plateThickness], center = true);

        // Define the text layout
        textLines = [  // 11 cols, 10 rows
            "ESKISTAFÜNF",
            "ZEHNZWANZIG",
            "DREIVIERTEL",
            "VORFUNKNACH",
            "HALBAELFÜNF",
            "EINSXAMZWEI",
            "DREIPMJVIER",
            "SECHSNLACHT",
            "SIEBENZWÖLF",
            "ZEHNEUNKUHR"
        ];


        // Calculate total text grid dimensions
        textWidth = len(textLines[0]) * (fontSize + spacing) - spacing;
        textHeight = len(textLines) * (fontSize + spacing) - spacing;

        // Generate text cutouts
        for (row = [0 : len(textLines) - 1]) {
            for (col = [0 : len(textLines[row]) - 1]) {
                translate([
                    -textWidth / 2 + col * (fontSize + spacing),
                    textHeight / 2 - row * (fontSize + spacing),
                    0
                ])
                linear_extrude(height = plateThickness)
                    text(
                        textLines[row][col],
                        size = fontSize,
                        valign = "center",
                        halign = "center",
                        font= "Wordclock Stencil Mono"
                    );
            }
        }
    }
}


// Call the module to render the front plate
// FrontPlate();

module Diffuser(diffuserThickness = 5, diffuserLength = 370, diffuserWidth = 370) {
    cube([diffuserLength, diffuserWidth, diffuserThickness], center = true);
}

module Separator(separatorThickness = 25, separatorLength = 450) {
    cube([separatorLength, separatorLength, separatorThickness], center = true);

    // Define the number of rows and columns for the holes
    rows = 10;
    cols = 11;

    // Calculate spacing between holes
    holeSpacingX = separatorLength / cols;
    holeSpacingY = separatorLength / rows;

    // Define hole radius
    holeRadius = 5;

    // Create the cylindrical holes
    for (row = [0 : rows - 1]) {
        for (col = [0 : cols - 1]) {
            translate([
                -separatorLength / 2 + (col + 0.5) * holeSpacingX,
                -separatorLength / 2 + (row + 0.5) * holeSpacingY,
                0
            ])
            rotate([90, 0, 0])
                cylinder(h = separatorThickness, r = holeRadius, center = true);
        }
    }
}

Separator();
