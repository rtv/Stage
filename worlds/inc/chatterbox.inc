# Define a model based on the Autonomy Lab Chatterbox robot
#
# Author: Richard T Vaughan (rtv) 
# $Id: chatterbox.inc,v 1.5 2008-01-15 01:25:42 rtv Exp $

include "irobot.inc"

define chatterbox create
(
  # long range IRs
  ranger
  (	
    scount 3
    spose[0] [  0.050  0.0   0 ]
    spose[1] [  0.0    0.050  90 ]
    spose[2] [  0.0   -0.050 270 ]
    sview [ 0 1.0 20 ]
    ssize [0.01 0.03 ]
  )

  # short rage IRs	
  ranger
  (
    scount 4
    spose[0] [  0.050  0.0   0 ]
    spose[1] [  0.035  0.035  30 ]
    spose[2] [ -0.050  0.0 180 ]
    spose[3] [  0.035 -0.035 330 ]

    sview [ 0 0.5 30 ]
    ssize [0.01 0.03 ]
  )
) 