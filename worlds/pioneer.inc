
# Desc: Device definitions for Activemedia Pioneer robots.
# Author: Andrew Howard
# Date: 10 Jun 2002
# CVS: $Id: pioneer.inc,v 1.3 2002-10-25 20:14:44 gerkey Exp $

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
)
