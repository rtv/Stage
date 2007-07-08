
# Desc: Device definitions for Activemedia robots.
# Author: Andrew Howard, Richard Vaughan
# Date: 10 Jun 2002
# CVS: $Id: pioneer.inc,v 1.27.4.5 2007-07-08 02:08:53 rtv Exp $


# The Pioneer2DX sonar array
define p2dx_sonar model # ranger
(
  size [ 0.40 0.33 0.06 ] # matches the Pioneer 2-DX dimensions

  scount 16 # the number of transducers

  # define the pose of each transducer [xpos ypos heading]
  spose[0] [ 0.0075 0.0130 90 ]
  spose[1] [ 0.0115 0.0115 50 ]
  spose[2] [ 0.0150 0.0080 30 ]
  spose[3] [ 0.0170 0.0025 10 ]
  spose[4] [ 0.0170 -0.0025 -10 ]
  spose[5] [ 0.0150 -0.0080 -30 ]
  spose[6] [ 0.0115 -0.0115 -50 ]
  spose[7] [ 0.0075 -0.0130 -90 ]
  spose[8] [ -0.0155 -0.0130 -90 ]
  spose[9] [ -0.0195 -0.0115 -130 ]
  spose[10] [ -0.0230 -0.0080 -150 ]
  spose[11] [ -0.0250 -0.0025 -170 ]
  spose[12] [ -0.0250 0.0025 170 ]
  spose[13] [ -0.0230 0.0080 150 ]
  spose[14] [ -0.0195 0.0115 130 ]
  spose[15] [ -0.0155 0.0130 90 ]	
		
  # define the field of view of each transducer [range_min range_max view_angle]
  sview [0 5.0 15]

  # define the size of each transducer [xsize ysize] in meters
  ssize [0.01 0.05]

 # this block approximates the shape of a pioneer
)

 
define lid model 
( 
  size [0.44 0.38 0.01] # sizes from MobileRobots' web site 	   

  blocks 1
  block[0].points 8
  block[0].point[0] [  0.23  0.08 ]
  block[0].point[1] [  0.18  0.15 ]
  block[0].point[2] [ -0.18  0.15 ]
  block[0].point[3] [ -0.23  0.08 ]
  block[0].point[4] [ -0.23 -0.08 ]
  block[0].point[5] [ -0.18 -0.15 ]
  block[0].point[6] [  0.18 -0.15 ]
  block[0].point[7] [  0.23 -0.08 ]
)



# a Pioneer 2 or 3 in standard configuration
define pioneer2dx position
(
  # actual size
  size [0.38 0.33 0.16] # sizes from MobileRobots' web site

  # the pioneer's center of rotation is offset from its center of area
  origin [-0.04 0 0 0]

  # draw a nose on the robot so we can see which way it points
  gui_nose 1

  # estimated mass in KG
  mass 15.0 

 # this set of blocks approximates the shape of a pioneer

  blocks 6

  # main body
  block[0].points 8
  block[0].point[0] [  0.14  0.07 ]
  block[0].point[1] [  0.12  0.115 ]
  block[0].point[2] [ -0.18  0.115 ]
  block[0].point[3] [ -0.20  0.07 ]
  block[0].point[4] [ -0.20 -0.07 ]
  block[0].point[5] [ -0.18 -0.115 ]
  block[0].point[6] [  0.12 -0.115 ]
  block[0].point[7] [  0.14 -0.07 ]
  block[0].height 0.12
  block[0].z_offset 0.04

  # left wheel
  block[1].points 4
  block[1].point[0] [  0.08  0.16 ]
  block[1].point[1] [ -0.08  0.16 ]
  block[1].point[2] [ -0.08  0.12 ]
  block[1].point[3] [  0.08  0.12 ]
  block[1].height 0.16
  block[1].z_offset 0.0

  # right wheel
  block[2].points 4
  block[2].point[0] [  0.08  -0.12 ]
  block[2].point[1] [ -0.08  -0.12 ]
  block[2].point[2] [ -0.08  -0.16 ]
  block[2].point[3] [  0.08  -0.16 ]
  block[2].height 0.16
  block[2].z_offset 0.0

  # castor
  block[3].points 4
  block[3].point[3] [ -0.22  0.02 ]
  block[3].point[2] [ -0.16  0.02 ]
  block[3].point[1] [ -0.16 -0.02 ]
  block[3].point[0] [ -0.22 -0.02 ]
  block[3].height 0.04
  block[3].z_offset 0.0
 
  # sonar case
  block[5].points 12
  block[5].point[11] [ 0.115 0.115 ]
  block[5].point[10] [ 0.150 0.080 ]
  block[5].point[9] [ 0.170 0.025 ]
  block[5].point[8] [ 0.170 -0.025  ]
  block[5].point[7] [ 0.150 -0.080  ]
  block[5].point[6] [ 0.115 -0.115  ]
  block[5].point[5] [ -0.195 -0.115  ]
  block[5].point[4] [ -0.230 -0.080  ]
  block[5].point[3] [ -0.250 -0.025  ]
  block[5].point[2] [ -0.250 0.025 ]
  block[5].point[1] [ -0.230 0.080 ]
  block[5].point[0] [ -0.195 0.115  ]
  block[5].height 0.05
  block[5].z_offset 0.16

  # lid
  block[4].points 16
  block[4].point[0] [  0.18  0.0 ]
  block[4].point[1] [  0.17  0.07 ]
  block[4].point[2] [  0.14  0.12 ]
  block[4].point[3] [  0.09  0.16 ]
  block[4].point[4] [  -0.08 0.16 ]
  block[4].point[5] [  -0.10  0.14 ]
  block[4].point[6] [ -0.20  0.14 ]
  block[4].point[7] [ -0.24  0.09 ]

  block[4].point[8] [ -0.26  0.0 ]

  block[4].point[9] [ -0.24 -0.09 ]
  block[4].point[10] [ -0.20 -0.14 ]
  block[4].point[11] [  -0.10  -0.14 ]
  block[4].point[12] [  -0.08 -0.16 ]

  block[4].point[13] [  0.09  -0.16 ]
  block[4].point[14] [  0.14  -0.12 ]
  block[4].point[15] [  0.17  -0.07 ]


  block[4].height 0.01
  block[4].z_offset 0.21


  # use the sonar array defined above
  #p2dx_sonar() 

  # differential steering model
  drive "diff"

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

