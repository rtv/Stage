
# Desc: Device definitions for handy beacons
# Author: Richard Vaughan
# Date: 29 October 2002
# CVS: $Id: beacons.inc,v 1.3 2004-07-02 01:37:30 rtv Exp $

# vision beacons - bright round circles
# that show up nicely in the blobfinder

define visionbeacon model
(
  color "orange" 
  size [0.2 0.2]
)


# laser beacons - laser-bright flat targets. any non-zero ID will cause
# this these objects to show up in the laserbeacondevice

define laserbeacon model
(
  fiducial.return 1
  laser.return "bright"
  size [ 0.05 0.3 ]
  color "LightBlue"
)

