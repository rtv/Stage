# Define a model based on the Autonomy Lab Chatterbox robot
#
# Author: Richard T Vaughan (rtv) 
# $Id: chatterbox.inc,v 1.2 2004-07-24 00:39:51 rtv Exp $

define chatterbox model
(
  ranger.count 8
  ranger.pose[0] [  0.050  0.0   0 ]
  ranger.pose[1] [  0.035  0.035  45 ]
  ranger.pose[2] [  0.0    0.050  90 ]
  ranger.pose[3] [ -0.035  0.035 135 ]
  ranger.pose[4] [ -0.050  0.0 180 ]
  ranger.pose[5] [ -0.035 -0.035 225 ]
  ranger.pose[6] [  0.0   -0.050 270 ]
  ranger.pose[7] [  0.035 -0.035 315 ]

  ranger.view[0] [ 0 0.5 0 ]
  ranger.view[1] [ 0 0.5 0 ]
  ranger.view[2] [ 0 0.5 0 ]
  ranger.view[3] [ 0 0.5 0 ]
  ranger.view[4] [ 0 0.5 0 ]
  ranger.view[5] [ 0 0.5 0 ]
  ranger.view[6] [ 0 0.5 0 ]
  ranger.view[7] [ 0 0.5 0 ]

  # mass in KG
  mass 0.2

  size [0.10 0.10]
  nose 1

  energy.capacity 200.0
  energy.range 0.8
  energy.return 0

  # fiducialfinder configuration
  # format [ minrange maxrange fov ]
  fiducial.view [ 0.0 2.0 360 ]

  # blobfinder configuration
  blob.bounds [ 0 8.0 ]  
  blob.ptz [ 0 0 60 ]
  blob.count 6
  blob.channels [ "red" "green" "blue" "cyan" "yellow" "magenta" ]	
) 