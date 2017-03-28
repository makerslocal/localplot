# Roadmap
<!-- Where we're going, we don't need 5-eyes. -->

### Past

#### v0.4.0

Total rewrite based on storing HPGL data in QGraphicsScene rather than custom classes.

* UI simplified and cleaned.
* Worker thread implemented (poorly).
* Multiple HPGL files can be opened and plotted.
* Cancel/Pause active cuts.
* Progress bar (seperate UI thread).
* Salience for serial connection status.
* ETA (cut time estimation).
* Set size of vinyl

#### v0.3.1

Some bugfixes and minor features:

* General UI cleaning.
	* Better support for Qt settings construct.
* Incremental serial output for USCutter MH871-MK2.

#### v0.3 - "Flexible"

* Have basic transformations working. Translate, scale, rotate. All via methods available in the classes that handle hpgl.

#### v0.2 - "Classy"

* Program at this stage should read AND parse hpgl code. The opcodes that can be accepted are based on those most commonly output by Inkscape (example: Inkscape exports all circles as line segments, even though hpgl as a standard has an arc opcode).
* The classes for working with hpgl should be 'kinda' robust.

#### v0.1 - "Does stuff"

* Program can read an hpgl file, cache the commands, and spit them back out at a serial port. Literally nothing else happens, this is essentially a fancy replacement for copy-pasting the hpgl code to a serial buffer.

### Future

#### v0.5.0

Rewrite that switches from list to data model. Smaller, more task specific threads.

* Auto arrange (bin packing).
* Run perimeter command.
* Rotations/scaling on individual hpgl files.

#### Unsorted ToDo:

* Generate cutout boxes.
* Alert when done.
* Automatically attempt to parse non hpgl files (inkscape script, imagemagick, gimp?).
* Export to USB plotter.
* Export to printer.
* Tiling (QBrush) grid background
* Side rulers?

<br><br><br><br>



