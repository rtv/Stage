# Desc: Device definitions for P3DX Activemedia robots.
# Author: Luis Riazuelo riazuelo@unizar.es
# Date: 13 Jun 2007

# defines sick laser
include "pioneerBase.inc"
include "sick.inc"

# The Pioneer3DX sonar array
define p3dx_sonar ranger
(
  scount 16

  # define the pose of each transducer [xpos ypos heading]
  spose[0] [ 0.069 0.136 90 ]
  spose[1] [ 0.114 0.119 50 ]
  spose[2] [ 0.148 0.078 30 ]
  spose[3] [ 0.166 0.027 10 ]
  spose[4] [ 0.166 -0.027 -10 ]
  spose[5] [ 0.148 -0.078 -30 ]
  spose[6] [ 0.114 -0.119 -50 ]
  spose[7] [ 0.069 -0.136 -90 ]
  spose[8] [ -0.157 -0.136 -90 ]
  spose[9] [ -0.203 -0.119 -130 ]
  spose[10] [ -0.237 -0.078 -150 ]
  spose[11] [ -0.255 -0.027 -170 ]
  spose[12] [ -0.255 0.027 170 ]
  spose[13] [ -0.237 0.078 150 ]
  spose[14] [ -0.103 0.119 130 ]
  spose[15] [ -0.157 0.136 90 ]	
		
  # define the field of view of each transducer [range_min range_max view_angle]
  sview [0.1 5.0 30]  # min (m), max (m), field of view (deg)

  # define the size of each transducer [xsize ysize] in meters
  ssize [0.01 0.04]
)

# The Pioneer3DX standard configuration
define pioneer3dx pioneerBase
(
  # Actual size
  size [0.511 0.4]

  # The pioneer's center of rotation is offset from its center of area
  origin [-0.04465 0.0 0.0]

  # Estimated mass in KG
  mass 23.0 

  # Body shape:
  polygons 1
  polygon[0].points 8
  polygon[0].point[0] [-0.12 0.2555]
  polygon[0].point[1] [0.12 0.2555]
  polygon[0].point[2] [0.2 0.12]
  polygon[0].point[3] [0.2 -0.12]
  polygon[0].point[4] [0.12 -0.2555]
  polygon[0].point[5] [-0.12 -0.2555]
  polygon[0].point[6] [-0.2 -0.12]
  polygon[0].point[7] [-0.2 0.12]

  # Use the sonar array defined above
  p3dx_sonar()  

  # Use the laser lms200
  sicklms200( pose [0.12 0 0] )

) 


