
# Desc: Device definitions for typical USC robot configurations
# Author: Richard Vaughan, Andrew Howard
# Date: 3 July 2002
# CVS: $Id: usc_pioneer.inc,v 1.2 2002-07-17 00:21:44 rtv Exp $

# use the ActivMedia Pioneer definitions
#
include "pioneer.inc"

define usc_pioneer position
(
  name "usc_pioneer"
  bps()
  p2dx_sonar()
  gripper ( pose [0.2 0 0] consume "true" )
  laser 
  (
    lbd () 
    ptz ( lens "wide" vision())
  )
)
