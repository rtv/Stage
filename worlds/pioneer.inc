
# Desc: Device definitions for Activemedia robots.
# Author: Richard Vaughan, Andrew Howard
# Date: 10 Jun 2002
# CVS: $Id: pioneer.inc,v 1.27.4.14 2007-12-29 05:07:40 rtv Exp $


# The Pioneer2DX sonar array
define p2dx_sonar ranger
(
  #size3 [ 0.40 0.33 0.06 ] # matches the Pioneer 2-DX dimensions

  scount 16 # the number of transducers

  # define the pose of each transducer [xpos ypos heading]
  spose[0] [ 0.075 0.130 90 ]
  spose[1] [ 0.115 0.115 50 ]
  spose[2] [ 0.150 0.080 30 ]
  spose[3] [ 0.170 0.025 10 ]
  spose[4] [ 0.170 -0.025 -10 ]
  spose[5] [ 0.150 -0.080 -30 ]
  spose[6] [ 0.115 -0.115 -50 ]
  spose[7] [ 0.075 -0.130 -90 ]
  spose[8] [ -0.155 -0.130 -90 ]
  spose[9] [ -0.195 -0.115 -130 ]
  spose[10] [ -0.230 -0.080 -150 ]
  spose[11] [ -0.250 -0.025 -170 ]
  spose[12] [ -0.250 0.025 170 ]
  spose[13] [ -0.230 0.080 150 ]
  spose[14] [ -0.195 0.115 130 ]
  spose[15] [ -0.155 0.130 90 ]	
		
  # define the field of view of each transducer [range_min range_max view_angle]
  sview [0 5.0 15]

  # define the size of each transducer [xsize ysize] in meters
  ssize [0.01 0.05]
)


define pioneer2dx position
(
  # actual size
  size3 [0.44 0.38 0.22] # sizes from MobileRobots' web site

  # the pioneer's center of rotation is offset from its center of area
  origin3 [-0.04 0 0 0]

  # draw a nose on the robot so we can see which way it points
  gui_nose 1

  # estimated mass in KG
  mass 15.0 

  # use the sonar array defined above with a small vertical offset to
  # drop the sensors into the robot body
  p2dx_sonar( pose4 [.04 0 -0.05 0] ) 

  # differential steering model
  drive "diff"
  
  # this polygon approximates the shape of a pioneer
  blocks 1
  block[0].points 8
  block[0].point[0] [  0.23  0.05 ]
  block[0].point[1] [  0.15  0.15 ]
  block[0].point[2] [ -0.15  0.15 ]
  block[0].point[3] [ -0.23  0.05 ]
  block[0].point[4] [ -0.23 -0.05 ]
  block[0].point[5] [ -0.15 -0.15 ]
  block[0].point[6] [  0.15 -0.15 ]
  block[0].point[7] [  0.23 -0.05 ]
  block[0].z [0 0.45]
)

# a Pioneer 2 or 3 in standard configuration
define fancypioneer2dx pioneer2dx
(
  # this set of blocks approximates the shape of a real Pioneer

  #  The geometry is from the Webots v5.3.0 manual. Thanks to Webots
  #  and Olivier Michel. If Stage or Gazebo do not do what you want,
  #  take a look at Webots. It's a very nice commercial simulator.

  blocks 7
  # main body
  block[0].points 8
  block[0].point[7] [ -0.215 -0.1   ] 
  block[0].point[6] [ -0.215  0.1   ]
  block[0].point[5] [ -0.185  0.135 ]
  block[0].point[4] [  0.095  0.135 ] 
  block[0].point[3] [  0.11   0.08  ] 
  block[0].point[2] [  0.11  -0.08  ] 
  block[0].point[1] [  0.095 -0.135 ] 
  block[0].point[0] [ -0.185 -0.135 ] 
  block[0].z [ 0.059 0.234 ]


  # sonar case
  block[1].points 9
  block[1].point[0]  [ -0.135  0.136 ]
  block[1].point[1]  [ -0.185  0.136 ]
  block[1].point[2]  [ -0.223  0.101 ]
  block[1].point[3]  [ -0.248  0.054 ]
  block[1].point[4]  [ -0.258  0     ]
  block[1].point[5]  [ -0.248 -0.054 ]
  block[1].point[6]  [ -0.223 -0.101 ]
  block[1].point[7]  [ -0.185 -0.136 ]
  block[1].point[8]  [ -0.135 -0.136 ]
  block[1].z [ 0.184 0.234 ]

  #sonar case
  block[2].points 9
  block[2].point[0]  [ 0.046 -0.136 ]
  block[2].point[1]  [ 0.096 -0.136 ]
  block[2].point[2]  [ 0.134 -0.101 ]
  block[2].point[3]  [ 0.159 -0.054 ]
  block[2].point[4]  [ 0.168  0     ]
  block[2].point[5]  [ 0.159  0.054 ]
  block[2].point[6]  [ 0.134  0.101 ]
  block[2].point[7]  [ 0.096  0.136 ]
  block[2].point[8]  [ 0.046  0.136 ]
  block[2].z [ 0.184 0.234 ]

  # left wheel
  block[3].points 4
  block[3].point[0] [  0.083  0.177 ]
  block[3].point[1] [ -0.083  0.177 ]
  block[3].point[2] [ -0.083  0.140 ]
  block[3].point[3] [  0.083  0.140 ]
  block[3].z [0 0.165 ]
  block[3].color "gray10"

  # right wheel
  block[4].points 4
  block[4].point[0] [  0.083  -0.14 ]
  block[4].point[1] [ -0.083  -0.14 ]
  block[4].point[2] [ -0.083  -0.177 ]
  block[4].point[3] [  0.083  -0.177 ]
  block[4].z [ 0 0.165 ]
  block[4].color "gray10"

  # castor
  block[5].points 4
  block[5].point[3] [ -0.2475  0.012 ]
  block[5].point[2] [ -0.1825  0.012 ]
  block[5].point[1] [ -0.1825 -0.012 ]
  block[5].point[0] [ -0.2475 -0.012 ]
  block[5].z [ 0 0.065 ]
  block[5].color "gray10"

  # lid
  block[6].points 22
  block[6].point[21]  [  0.174 0 ]
  block[6].point[20]  [  0.166 -0.056 ]
  block[6].point[19]  [  0.145 -0.107 ]
  block[6].point[18]  [  0.112 -0.155 ]
  block[6].point[17]  [  0.064 -0.190 ]
  block[6].point[16]  [  -0.074 -0.190 ]
  block[6].point[15]  [  -0.096 -0.160 ]
  block[6].point[14]  [  -0.151 -0.160 ]
  block[6].point[13]  [  -0.2   -0.155 ]
  block[6].point[12]  [  -0.236 -0.107 ]
  block[6].point[11] [  -0.256 -0.056 ]
  block[6].point[10] [  -0.264  0     ]
  block[6].point[9] [  -0.256  0.056 ]
  block[6].point[8] [ -0.236  0.107 ]
  block[6].point[7] [ -0.2    0.155 ]
  block[6].point[6] [ -0.151  0.160 ]
  block[6].point[5] [ -0.096  0.160 ]
  block[6].point[4] [ -0.074  0.190 ]
  block[6].point[3] [  0.064  0.190 ]
  block[6].point[2] [  0.112  0.155 ]
  block[6].point[1] [  0.145  0.107 ]
  block[6].point[0] [  0.166  0.056 ]
  block[6].z [ 0.234 0.24 ]
  # a dark top looks more realistic, but isn't very useful 
  # for a top-down view
  #block[6].color "gray10"
) 

# The AmigoBot sonar array
define amigo_sonar ranger
(
  scount 8
  spose[0] [ 0.073 0.105 90 ]
  spose[1] [ 0.130 0.078 41 ]
  spose[2] [ 0.154 0.030 15 ]
  spose[3] [ 0.154 -0.030 -15 ]
  spose[4] [ 0.130 -0.078 -41 ]	
  spose[5] [ 0.073 -0.105 -90 ]
  spose[6] [ -0.146 -0.060 -145 ]
  spose[7] [ -0.146 0.060 145 ]
)

define amigobot position
(
  size [.330 .280 0.25]
  origin [0 0 0 0] # what should this value be? send email to vaughan@sfu.ca.
  amigo_sonar()
)


# define 10 straight bumpers around the edge of the robot
#
# (these angles are correct for p2dx but the offsets are approximate - RTV)
# format: bumper[x] [x y th length radius] (zero radius gives a straight line)
# WARNING: bumpers are not currently supported by Stage>=1.5
# define pioneer2dxbumper bumper
# ( 
#   bumpers10
#   bumper[0] [  0.17 -0.22  -52  0.105 0.0 ]
#   bumper[1] [  0.24 -0.12  -19  0.105 0.0 ]
#   bumper[2] [  0.26  0.00    0  0.105 0.0 ]
#   bumper[3] [  0.24  0.12   19  0.105 0.0 ]
#   bumper[4] [  0.17  0.22   52  0.105 0.0 ]
#   bumper[5] [ -0.25  0.22  128  0.105 0.0 ]
#   bumper[6] [ -0.32  0.12  161  0.105 0.0 ]
#   bumper[7] [ -0.34  0.00  180  0.105 0.0 ]
#   bumper[8] [ -0.32 -0.12  199  0.105 0.0 ]
#   bumper[9] [ -0.25 -0.22  232  0.105 0.0 ]
# )

