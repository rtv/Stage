
# Desc: Device definitions for Activemedia Pioneer robots.
# Author: Andrew Howard
# Date: 10 Jun 2002
# CVS: $Id: pioneer.inc,v 1.9 2004-07-02 01:37:30 rtv Exp $

# The Pioneer2DX sonar array


# The Pioneer2DX sonar array (new format)
define p2dx_sonar_new sonar
(
  ranger.count 16
  ranger.pose[0] [ 0.115 0.130 90 ]
  ranger.pose[1] [ 0.155 0.115 50 ]
  ranger.pose[2] [ 0.190 0.080 30 ]
  ranger.pose[3] [ 0.210 0.025 10 ]
  ranger.pose[4] [ 0.210 -0.025 -10 ]
  ranger.pose[5] [ 0.190 -0.080 -30 ]
  ranger.pose[6] [ 0.155 -0.115 -50 ]
  ranger.pose[7] [ 0.115 -0.130 -90 ]
  ranger.pose[8] [ -0.115 -0.130 -90 ]
  ranger.pose[9] [ -0.155 -0.115 -130 ]
  ranger.pose[10] [ -0.190 -0.080 -150 ]
  ranger.pose[11] [ -0.210 -0.025 -170 ]
  ranger.pose[12] [ -0.210 0.025 170 ]
  ranger.pose[13] [ -0.190 0.080 150 ]
  ranger.pose[14] [ -0.155 0.115 130 ]
  ranger.pose[15] [ -0.115 0.130 90 ]

  #ssize[0] [0 0.3]
)

# the pioneer's center of rotation is offset from its 
# center of area 
#
define pioneer2dx position
(
  size [0.44 0.33]
  #offset [-0.04 0.0]
  #p2dx_sonar()
  #power()
  
  nose 1

  laser.samples 180

  # laser view vector - format: [ minrange maxrange fov ]
  laser.view [ 0.0 8.0 180.0 ]
	
  laser.pose [ 0 0 0 ]
  laser.size [ 0.15 0.15 ]
	
  # fiducialfinder view vector - format [ minrange maxrange fov ]
  fiducial.view [ 0.0 5.0 90.0 ]  
  fiducial.id_limit 4.0

  # blobfinder configuration 
  blob.count 6
  blob.screen [ 80 60 ]

  # blob view vector - format [minrange maxrange fov]	
  blob.view [ 0 7.0 60 ]	
  blob.channels [ "red" "green" "blue" "cyan" "yellow" "magenta" ]	

  energy.capacity 3000.0
  energy.range 1.0
  energy.return 0 # we don't give charge

  # mass in KG
  mass 15.0 

  lines.count 8
  lines.points[0] [0.23 0.05 0.15 0.15]
  lines.points[1] [0.15 0.15 -0.15 0.15]
  lines.points[2] [-0.15 0.15 -0.23 0.05]
  lines.points[3] [-0.23 0.05 -0.23 -0.05]
  lines.points[4] [-0.23 -0.05 -0.15 -0.15]
  lines.points[5] [-0.15 -0.15 0.15 -0.15]
  lines.points[6] [0.15 -0.15 0.23 -0.05]
  lines.points[7] [0.23 -0.05 0.23 0.05] 

  ranger.count 16
  ranger.pose[0] [ 0.115 0.130 90 ]
  ranger.pose[1] [ 0.155 0.115 50 ]
  ranger.pose[2] [ 0.190 0.080 30 ]
  ranger.pose[3] [ 0.210 0.025 10 ]
  ranger.pose[4] [ 0.210 -0.025 -10 ]
  ranger.pose[5] [ 0.190 -0.080 -30 ]
  ranger.pose[6] [ 0.155 -0.115 -50 ]
  ranger.pose[7] [ 0.115 -0.130 -90 ]
  ranger.pose[8] [ -0.115 -0.130 -90 ]
  ranger.pose[9] [ -0.155 -0.115 -130 ]
  ranger.pose[10] [ -0.190 -0.080 -150 ]
  ranger.pose[11] [ -0.210 -0.025 -170 ]
  ranger.pose[12] [ -0.210 0.025 170 ]
  ranger.pose[13] [ -0.190 0.080 150 ]
  ranger.pose[14] [ -0.155 0.115 130 ]
  ranger.pose[15] [ -0.115 0.130 90 ]
) 

# define 10 straight bumpers around the edge of the robot
#
# (these angles are correct for p2dx but the offsets are approximate - RTV)
# bpose format: [x y th length radius] (zero radius gives a straight line)
#
define pioneer2dxbumper bumper
( 
  bumper.count 10
  bumper.pose[0] [  0.17 -0.22  -52  0.105 0.0 ]
  bumper.pose[1] [  0.24 -0.12  -19  0.105 0.0 ]
  bumper.pose[2] [  0.26  0.00    0  0.105 0.0 ]
  bumper.pose[3] [  0.24  0.12   19  0.105 0.0 ]
  bumper.pose[4] [  0.17  0.22   52  0.105 0.0 ]
  bumper.pose[5] [ -0.25  0.22  128  0.105 0.0 ]
  bumper.pose[6] [ -0.32  0.12  161  0.105 0.0 ]
  bumper.pose[7] [ -0.34  0.00  180  0.105 0.0 ]
  bumper.pose[8] [ -0.32 -0.12  199  0.105 0.0 ]
  bumper.pose[9] [ -0.25 -0.22  232  0.105 0.0 ]
)

# The AmigoBot sonar array
define amigo_sonar sonar
(
  ranger.count 8
  ranger.pose[0] [ 0.073 0.105 90 ]
  ranger.pose[1] [ 0.130 0.078 41 ]
  ranger.pose[2] [ 0.154 0.030 15 ]
  ranger.pose[3] [ 0.154 -0.030 -15 ]
  ranger.pose[4] [ 0.130 -0.078 -41 ]
  ranger.pose[5] [ 0.073 -0.105 -90 ]
  ranger.pose[6] [ -0.146 -0.060 -145 ]
  ranger.pose[7] [ -0.146 0.060 145 ]
)

define amigobot position
(
  size [.330 .280]
  # what should the offset be?
  #offset [0.0 0.0]
  amigo_sonar()
  power()
)
