
# Desc: Device definitions for typical USC robot configurations
# Author: Richard Vaughan, Andrew Howard
# Date: 3 July 2002
# CVS: $Id: usc_pioneer.inc,v 1.1 2002-07-04 01:06:02 rtv Exp $

# use the ActivMedia Pioneer definitions
#
include "pioneer.inc"

define usc_pioneer position
(
  #name "usc_pioneer"
  bps()
  #p2dx_sonar()
  gripper ( pose [.20 0 0] consume "true" )
  laser 
  (
    lbd () 
    ptz ( lens "wide" vision())
  )
)
