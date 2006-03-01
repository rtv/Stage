# Define a model based on the Autonomy Lab Chatterbox robot
#
# Author: Richard T Vaughan (rtv) 
# $Id: chatterbox.inc,v 1.2 2006-03-01 19:24:43 rtv Exp $

define chatterbox position
(
  ranger(	
    scount 8
    spose[0] [  0.050  0.0   0 ]
    spose[1] [  0.050  0.0   15 ]
    spose[2] [  0.035  0.035  30 ]
    spose[3] [  0.0    0.050  90 ]
    spose[4] [ -0.050  0.0 180 ]
    spose[5] [  0.0   -0.050 270 ]
    spose[6] [  0.035 -0.035 330 ]
    spose[7] [  0.050 -0.000 345 ]

    sview [ 0 0.5 15 ]
    ssize [0.01 0.03 ]
  )

  mass 0.5
  size [0.10 0.10]
  gui_nose 1
) 