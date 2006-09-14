# Define a model based on the Autonomy Lab Chatterbox robot
#
# Author: Richard T Vaughan (rtv) 
# $Id: chatterbox.inc,v 1.4.4.1 2006-09-14 07:11:55 rtv Exp $

define chatterbox position
(
  # long range IRs
  ranger(	
    base -0.1	
    scount 3
    spose[0] [  0.050  0.0   0 ]
    spose[1] [  0.0    0.050  90 ]
    spose[2] [  0.0   -0.050 270 ]

    sview [ 0 1.0 20 ]
    ssize [0.01 0.03 ]
  )

  # short rage IRs	
  ranger(
    base -0.1	
    scount 4
    spose[0] [  0.050  0.0   0 ]
    spose[1] [  0.035  0.035  30 ]
    spose[2] [ -0.050  0.0 180 ]
    spose[3] [  0.035 -0.035 330 ]

    sview [ 0 0.5 30 ]
    ssize [0.01 0.03 ]
  )


  mass 0.5
  size [0.12 0.12]
  height 0.12
  gui_nose 1

  polygon[0].fill 0
) 