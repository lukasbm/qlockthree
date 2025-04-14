const jscad = require('@jscad/modeling');
const { cuboid } = jscad.primitives;
const { vectorText, vectorChar } = jscad.text;
const { extrudeLinear } = jscad.extrusions;
const { translate, center } = jscad.transforms;
const { subtract } = jscad.booleans;

const createFrontPlate = (fontSize = 20, sideLength = 450, spacing = 10, margin = 50) => {
    const plateThickness = 2; // Thickness of the glass plate

    // Create the base plate
    const basePlate = cuboid({ size: [sideLength, sideLength, plateThickness] });

    // Define the text layout

    const minuteMarks = [`↖`, `↗`, `↘`, `↙`];

    // Generate text cutouts
    const textExtrusions = textLines.flatMap((line, rowIndex) => {
        const textShapes = vectorText({ x: 0, y: 0 }, line, { size: fontSize });
        return textShapes.map((shape, charIndex) => {
            const x = margin + charIndex * (fontSize + spacing);
            const y = sideLength - margin - rowIndex * (fontSize + spacing);
            const extrudedText = extrudeLinear({ height: plateThickness }, shape);
            return translate([x, y, 0], extrudedText);
        });
    });

    // Subtract text from the base plate
    const validTextExtrusions = textExtrusions.filter(shape => shape && shape.polygons && shape.polygons.length > 0);
    const frontPlate = subtract(basePlate, ...validTextExtrusions);
    return center({ axes: [true, true, false] }, frontPlate);
};


const createFrontPlateText = (font_size = 20) => {
    const textLines = [
        'ESKISTAFÜNF',
        'ZEHNZWANZIG',
        'DREIVIERTEL',
        'VORFUNKNACH',
        'HALBAELFÜNF',
        'EINSXAMZWEI',
        'DREIPMJVIER',
        'SECHSNLACHT',
        'SIEBENZWÖLF',
        'ZEHNEUNKUHR'
    ];

    // Create 2D outline
    const outlines = vectorText({ x: 0, y: 0 }, textLines, { size: font_size });

    // Convert to Shape
    const shapes = outlines.map(segment => polygon(segment));

    // Combine and extrude into 3D
    const text3D = union(...shapes).extrudeLinear({ height: 2 });

    return text3D;
}


const main = () => {
    // return createFrontPlate();
    return createFrontPlateText();
};

module.exports = { main };
