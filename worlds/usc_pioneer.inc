
# Desc: Device definitions for typical USC robot configurations
# Author: Richard Vaughan, Andrew Howard
# Date: 3 July 2002
# CVS: $Id: usc_pioneer.inc,v 1.7 2002-11-01 19:12:33 rtv Exp $

# use the ActivMedia Pioneer definitions
#
include "pioneer.inc"

define usc_pioneer pioneer2dx
(
  name "usc_pioneer"
  # bps device disabled for now
  #  bps()
  broadcast()
  laser 
  (
    lbd () 
   ptz ( lens "wide" vision())
  )
)

# add a P2DX bumper to the usc_pioneer
#
define usc_pioneer_bumper usc_pioneer
(
  pioneer2dxbumper()
)

# add a P2DX gripper to the usc_pioneer
#
define usc_pioneer_gripper usc_pioneer
(
  gripper ( pose [0.2 0 0] consume "true" )
)
