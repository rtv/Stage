############################################################################
# $Id: README.xs,v 1.1 2001-08-10 19:16:15 vaughan Exp $
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

cursor keys - pan
z - zoom in
x - zoom out
d - print a description of Stage's internal model on stdout
f - print a more verbose description of Stage's internal model on stdout

##########################################################################

xs understands the following X resources:

xs.geometry: 600x600
xs.channels: red green blue yellow magenta cyan
xs.zoom: 100
xs.pan: 10x10

You may want to copy these into your ~/.Xdefaults file.


