
# Desc: This is an include file with USC-specific devices.
# Author: Andrew Howard
# Date: 4 Jun 2002
# CVS: $Id: usc-devices.inc,v 1.1 2002-06-10 20:27:20 inspectorg Exp $

define sonar_p2dx sonar
(
  scount 2 
  spose[0] [0 0 0]
  spose[1] [0 0 180]
)

define robot_usc position
(
  player ()
  truth ()
  sonar_p2dx ()
  laser (pose [0.05 0 0])
)
