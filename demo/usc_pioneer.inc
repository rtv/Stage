
# Desc: Device definitions for typical USC robot configurations
# Author: Richard Vaughan, Andrew Howard
# Date: 3 July 2002
# CVS: $Id: usc_pioneer.inc,v 1.1 2002-07-30 00:39:50 rtv Exp $

# use the ActivMedia Pioneer definitions
#
include "pioneer.inc"

define usc_pioneer position
(
  name "USC Pioneer"

  laser 
  (
    pose [ 0.1 0 0 ]
    lbd () 
    ptz ( lens "wide" vision())
  )
)
