
# Desc: Device definitions for typical USC robot configurations
# Author: Richard Vaughan, Andrew Howard
# Date: 3 July 2002
# CVS: $Id: usc_pioneer.inc,v 1.3 2002-08-16 06:18:35 gerkey Exp $

# use the ActivMedia Pioneer definitions
#
include "pioneer.inc"

define usc_pioneer position
(
  name "usc_pioneer"
  bps()
  broadcast()
  p2dx_sonar()
  gripper ( pose [0.2 0 0] consume "true" )
  laser 
  (
    lbd () 
    ptz ( lens "wide" vision())
  )
)
