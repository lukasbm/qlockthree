import FreeCAD
import Draft
import os

work_dir = os.path.dirname(__file__)
print(work_dir)

# Access the active document and your "Parameters" spreadsheet
# (adjust the internal Name if needed)
doc = FreeCAD.ActiveDocument
ss = doc.getObject("Parameters") or doc.getObject("parameters") or doc.getObject("Spreadsheet")
if ss is None:
    raise ValueError("Spreadsheet 'Parameters' not found. Check its internal Name.")


# Define your word‑clock grid (10 rows of 11 characters each)
grid = [
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
]

# Pull spacing (pitch) from spreadsheet (millimeters)
dx = float(ss.led_spacing)
dy = float(ss.led_spacing)

# Provide full path to your .ttf font file that supports umlauts
font_file = os.path.join(work_dir, "fonts", "Oxanium", "static", "Oxanium-Regular.ttf")
# Display size of the font (height) in mm
txt_size = 22

# Create a group to hold all letter objects
group = doc.addObject("App::DocumentObjectGroup", "WordclockLetters")
origin = FreeCAD.Vector(0, 0, 0)

# Loop through grid, create each letter via ShapeString, center it in its cell, and add to group
for r, rowtext in enumerate(grid):
    for c, ch in enumerate(rowtext):
        cell_base = origin + FreeCAD.Vector(c * dx, -r * dy, 0)
        txt = Draft.make_shapestring(String=ch,
                                     FontFile=font_file,
                                     Size=txt_size,
                                     Tracking=0.0)
        # Ensure shape is up-to-date to compute bounding box
        doc.recompute()
        bbox = txt.Shape.BoundBox
        char_width = bbox.XLength
        x_offset = (dx - char_width) / 2.0
        placement = txt.Placement
        placement.Base = cell_base + FreeCAD.Vector(x_offset, 0, 0)
        txt.Placement = placement
        txt.Label = f"Letter_{r+1:02d}_{c+1:02d}_{ch}"
        group.addObject(txt)

# Recompute to finalize all shapes
doc.recompute()

# Compute bounding box of entire group by aggregating children
children = group.Group
minx = min(o.Shape.BoundBox.XMin for o in children)
maxx = max(o.Shape.BoundBox.XMax for o in children)
miny = min(o.Shape.BoundBox.YMin for o in children)
maxy = max(o.Shape.BoundBox.YMax for o in children)

# Calculate current center of the letters grid
center = FreeCAD.Vector((minx + maxx) / 2.0,
                        (miny + maxy) / 2.0,
                         0)
# Desired center (0,0,0) — adjust if your rectangle center differs
target = FreeCAD.Vector(0, 0, 0)

# Offset needed to move center to target
offset = target - center

# Apply translation to each letter in the group
for obj in children:
    plc = obj.Placement
    plc.Base = plc.Base + offset
    obj.Placement = plc

# Final recompute to apply centering
doc.recompute()
print("Done!")
