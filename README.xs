############################################################################
# $Id: README.xs,v 1.3.2.1 2001-08-17 20:48:03 vaughan Exp $
############################################################################

xs is an X GUI for stage that replaces the old xstage. It runs as an
independent process and communicates with Stage's truth server to
display and manipulate Stage's internal model.

To use it:

1) run stage 

% stage <yourworld.world> &

2) run xs

% xs

##########################################################################

xs understands the following keystrokes:

q - quit
(cursor keys) - pan 
z - zoom in
x - zoom out
d - print a description of Stage's internal model on stdout
f - print a more verbose description of Stage's internal model on stdout
p - create a postscript format dump of the window in the current directory
j - create a jpg format dump of the window in the current directory
v - toggle display of some usually-hidden objects, such as Players
s - request that Stage save the current state of the world in the .world file
    *warning: Stage adds all the current settings to each entity in your
    .world file. This can be very useful, but you might want to keep a copy
    of your easy-to-read orginal file before saving. 

##########################################################################

xs understands the following X resources:

xs.geometry: 600x600
xs.channels: red green blue yellow magenta cyan
xs.zoom: 1.0
xs.pan: 0x0 

- geometry is in standard X resource format.

- channels defines the colors that xs will draw for ACTS channels,
from 0, specified in color names from the X database.
 
- zoom is a floating point scaling factor.

- pan is x by y and is a percentage of the maximum possible pan.

You may want to copy these into your ~/.Xdefaults file.


