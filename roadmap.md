# Roadmap
<!-- Where we're going, we don't need 5-eyes. -->

### Past

#### v0.1

* Does stuff.
	* Program can read an hpgl file, cache the commands, and spit them back out at a serial port. Literally nothing else happens, this is essentially a fancy replacement for copy-pasting the hpgl code to a serial buffer.

### Future

#### TODO - unassigned

* run perimeter command
* pause active cuts
* Set size of vinyl

#### v0.2

* Classy
	* Program at this stage should read AND parse hpgl code. The opcodes that can be accepted are based on those most commonly output by Inkscape (example: Inkscape exports all circles as line segments, even though hpgl as a standard has an arc opcode).
	* The classes for working with hpgl should be 'kinda' robust.
	* Incremental serial output for USCutter MH871-MK2.

#### v0.3

* Flexible
	* Have basic transformations working. Translate, scale, rotate. All via methods available in the classes that handle hpgl.
	* Be able to create a box (with variable margin) around objects, to make removing from the vinyl reel easier.

#### v0.4

* Social
	* Should be able to represent and hold in objects multiple separate 'items'.
	* Each item should be able to have it's own transformations applied to it.
	* An item can be associated with a single hpgl file.

#### v0.5

* Twins
	* Items should be able to be duplicated.
	* Items should be able to be carved out of an hpgl file which has many items already.
	* Items should be able to be auto-arranged and grid-arranged.

##### v0.6

* Printer Talk
	* Program should be able to export to serial port and/or LP printer, as found on the USB connection on USCutter vinyl cutters.



<br><br><br><br>



