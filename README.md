# localplot
Qt HPGL plotting software


![Drew it maself.](https://raw.githubusercontent.com/makerslocal/localplot/master/images/logo.png  "Localplot Logo")

## What is this

Localplot is a small Qt tool we're writing to bridge the gap between Inkscape and our old USCutter vinyl cutter.

This program currently caters to a single plotting/cutting system: [USCutter MH871-MK2](http://www.uscutter.com/USCutter-MH-Series-Vinyl-Cutter-w-Sure-Cuts-A-Lot-Pro-Design-Cut-Software).

## Why is this

Our makerspace has one of the aforementioned vinyl cutters, and it's a fantastic community tool that works quite nicely.. When the software plays along. Right now we're using an ancient WinXP era driver program that's perpetually stuck in trial mode, it's a less-than-ideal situation.

We've tried the alternatives, all [one of them](http://inkcut.sourceforge.net/). There's a pretty hard absence of free and open source programs that can drive plotters, which I find crazy considering that the HPGL protocol is [older than digital dirt](http://cstep.luberth.com/hpgl.htm).

So we're here, creating a small utility which accepts .hpgl files from Inkscape, and then drives a vinyl cutter with them. Right now it does the bare minimum to make the magic happen, but I look forward to making this into a fully-fledged layout and arrangement tool.

## We can use your help

If there's a feature you'd like to see, tell us and we'll look into it.

If there's a bug, tell us and we'll (try to) remove it.

If there's not a bug, tell us and we'll add one.

