
# Desc: Device definitions for typical USC robot configurations
# Author: Richard Vaughan, Andrew Howard
# Date: 3 July 2002
# CVS: $Id: usc_pioneer.inc,v 1.6 2002-10-26 00:52:38 rtv Exp $

# use the ActivMedia Pioneer definitions
#
include "pioneer.inc"

define usc_pioneer pioneer2dx
(
  name "usc_pioneer"
  # bps device disabled for now
  #  bps()
  broadcast()
  gripper ( pose [0.2 0 0] consume "true" )
  laser 
  (
    lbd () 
   ptz ( lens "wide" vision())
  )
)
