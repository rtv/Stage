
# Desc: Device definitions for handy beacons
# Author: Richard Vaughan
# Date: 29 October 2002
# CVS: $Id: beacons.inc,v 1.4 2004-12-03 01:32:57 rtv Exp $

# laser beacons - flat targets that show up bright in the laser, and
# in the fiducial finder. It's often useful to redefine the
# fiducial_return property in each instance, to get unique landmark
# IDs.

define laserbeacon model 
(
  fiducial_return 1
  laser_return "bright"
  size [ 0.05 0.3 ]
  color "LightBlue"
)

# vision beacons - bright round squares that show up nicely in the
# blobfinder

define visionbeacon model
(
  size [0.2 0.2]  
)

# One version for each primary and secondary color.  These match the
# default settings for the blobfinder device

define visionbeacon_red visiobnbeacon
(
  color "red"
)

define visionbeacon_green visiobnbeacon
(
  color "green"
)

define visionbeacon_blue visiobnbeacon
(
  color "blue"
)

define visionbeacon_cyan visiobnbeacon
(
  color "cyan"
)

define visionbeacon_yellow visiobnbeacon
(
  color "yellow"
)

define visionbeacon_magenta visiobnbeacon
(
  color "magenta"
)


