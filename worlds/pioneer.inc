
# Desc: Device definitions for Activemedia robots.
# Author: Andrew Howard, Richard Vaughan
# Date: 10 Jun 2002
# CVS: $Id: pioneer.inc,v 1.27.4.1 2006-09-14 07:11:55 rtv Exp $


# The Pioneer2DX sonar array
define p2dx_sonar ranger
(
  size [ 0.44 0.38 ] # matches the Pioneer 3-DX dimensions

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
)

# a Pioneer 2 or 3 in standard configuration
define pioneer2dx position
(
  # actual size
  size3d [0.44 0.38 0.22] # sizes from MobileRobots' web site

  # the pioneer's center of rotation is offset from its center of area
  origin [-0.04 0.0 0]

  # draw a nose on the robot so we can see which way it points
  gui_nose 1

  # estimated mass in KG
  mass 15.0 
	
  # this polygon approximates the shape of a pioneer
  polygons 1
  polygon[0].points 8
  polygon[0].point[0] [  0.23  0.05 ]
  polygon[0].point[1] [  0.15  0.15 ]
  polygon[0].point[2] [ -0.15  0.15 ]
  polygon[0].point[3] [ -0.23  0.05 ]
  polygon[0].point[4] [ -0.23 -0.05 ]
  polygon[0].point[5] [ -0.15 -0.15 ]
  polygon[0].point[6] [  0.15 -0.15 ]
  polygon[0].point[7] [  0.23 -0.05 ]

  # differential steering model
  drive "diff"

  # uncomment this line if you want to model real pioneers with SICK
  # lasers, where the laser is taller than the robot
  # laser_return 0

  # use the sonar array defined above
  p2dx_sonar( origin3d [0 0 -0.1 0] )  
) 

#define pioneer2dx_battery pioneer2dx
#(
#  power( pose [-0.14 0 0] capacity 1000 probe_range 0.50  give 0 give_rate 100 take_rate 100 )
#)

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
  size [.330 .280]
  #origin [0.0 0.0] # what should this value be? send email to vaughan@sfu.ca.
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

