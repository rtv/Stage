
# Desc: Device definitions for Activemedia Pioneer robots.
# Author: Andrew Howard
# Date: 10 Jun 2002
# CVS: $Id: pioneer.inc,v 1.1 2002-06-11 06:08:47 inspectorg Exp $

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