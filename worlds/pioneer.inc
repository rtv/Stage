
# Desc: Device definitions for Activemedia Pioneer robots.
# Author: Andrew Howard
# Date: 10 Jun 2002
# CVS: $Id: pioneer.inc,v 1.5 2002-11-01 19:12:33 rtv Exp $

# The Pioneer2DX sonar array
define p2dx_sonar sonar
(
  scount 16
  spose[0] [ 0.115 0.130 90 ]
  spose[1] [ 0.155 0.115 50 ]
  spose[2] [ 0.190 0.080 30 ]
  spose[3] [ 0.210 0.025 10 ]
  spose[4] [ 0.210 -0.025 -10 ]
  spose[5] [ 0.190 -0.080 -30 ]
  spose[6] [ 0.155 -0.115 -50 ]
  spose[7] [ 0.115 -0.130 -90 ]
  spose[8] [ -0.115 -0.130 -90 ]
  spose[9] [ -0.155 -0.115 -130 ]
  spose[10] [ -0.190 -0.080 -150 ]
  spose[11] [ -0.210 -0.025 -170 ]
  spose[12] [ -0.210 0.025 170 ]
  spose[13] [ -0.190 0.080 150 ]
  spose[14] [ -0.155 0.115 130 ]
  spose[15] [ -0.115 0.130 90 ]
)

# the pioneer's center of rotation is offset from its 
# center of area 
#
define pioneer2dx position
(
  size [.440 .330]
  offset [-0.04 0.0]
  p2dx_sonar()
  power()
)

# define 10 straight bumpers around the edge of the robot
#
# (these angles are correct for p2dx but the offsets are approximate - RTV)
# bpose format: [x y th length radius] (zero radius gives a straight line)
#
define pioneer2dxbumper bumper
( 
  bcount 10
  bpose[0] [  0.17 -0.22  -52  0.105 0.0 ]
  bpose[1] [  0.24 -0.12  -19  0.105 0.0 ]
  bpose[2] [  0.26  0.00    0  0.105 0.0 ]
  bpose[3] [  0.24  0.12   19  0.105 0.0 ]
  bpose[4] [  0.17  0.22   52  0.105 0.0 ]
  bpose[5] [ -0.25  0.22  128  0.105 0.0 ]
  bpose[6] [ -0.32  0.12  161  0.105 0.0 ]
  bpose[7] [ -0.34  0.00  180  0.105 0.0 ]
  bpose[8] [ -0.32 -0.12  199  0.105 0.0 ]
  bpose[9] [ -0.25 -0.22  232  0.105 0.0 ]
)

# The AmigoBot sonar array
define amigo_sonar sonar
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
  # what should the offset be?
  #offset [0.0 0.0]
  amigo_sonar()
  power()
)
