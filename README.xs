############################################################################
# README.xs - instructions for using the XS Stage GUI - Richard Vaughan
#
# $Id: README.xs,v 1.4 2001-10-13 02:01:41 vaughan Exp $
############################################################################

!! This file is correct but incomplete. Check the code for more
options and interaction !!

XS is an X GUI for stage that replaces the old xstage. It runs as an
independent process and communicates with Stage's truth server to
display and manipulate Stage's internal model.

#-- Running XS ------------------------------------------------------------

XS will be build by default along with Stage.

To use it:

1) run stage (see README.stage)

% stage <worldfile> &

XS is run by default.

Stage can be run with XS disabled (stage -xs <worldfile>) and RTKStage
has xs disabled by default. To connect to a running Stage on the local
host, just start xs manually:
	
% xs &

To connect to Stage running on a remote machine, use the -h option

% xs -h <remote.host> &

#-- Mouse -----------------------------------------------------------------

xs understands the following mouse actions:

left click - grab the nearest top-level object
right click - toggle a Player client for the nearest top-level object

when an object is grabbed:

mouse move - move object
right click - rotate object clockwise
middle click - rotate oject anticlockwise
left click - release object

#-- Keys -------------------------------------------------------------------

xs understands the following keystrokes:

(cursor keys) - pan 
z - zoom in
x - zoom out
d - print a description of Stage's internal model on stdout
f - print a more verbose description of Stage's internal model on stdout
p - create a postscript format dump of the window in the current directory
j - create a jpg format dump of the window in the current directory
g - toggle drawing of a 1m grid over the world
s - command Stage to save the state of the world

##########################################################################
#-------------------------------------------------------------------------

xs understands the following X resources:

xs.geometry: 600x600
xs.zoom: 1.0
xs.pan: 0x0 

- geometry is in standard X resource format.

- channels defines the colors that xs will draw for ACTS channels,
from 0, specified in color names from the X database.
 
- zoom is a floating point scaling factor.

- pan is x by y and is a percentage of the maximum possible pan.

You may want to copy these into your ~/.Xdefaults file.

#-------------------------------------------------------------------------
RTV




