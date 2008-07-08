
define sicklaser laser
(
  # laser-specific properties

  # factory settings for LMS200	
  range_min 0.0
  range_max 8.0
  fov 180.0
  samples 361

  #samples 90 # still useful but much faster to compute

  # generic model properties
  color "blue"
  size [ 0.156 0.155 0.19 ] # dimensions from LMS200 data sheet	
)

define fancysicklaser sicklaser
(
  blocks 4

  # bottom
  block[0].points 4
  block[0].point[0] [ -0.02 -0.077 ]
  block[0].point[1] [  0.078 -0.077 ]
  block[0].point[2] [  0.078  0.077 ]
  block[0].point[3] [ -0.02  0.077 ]
  block[0].z [0 0.02 ]

  # back
  block[1].points 4
  block[1].point[0] [ -0.078 -0.077 ]
  block[1].point[1] [ -0.02  -0.077 ]
  block[1].point[2] [ -0.02   0.077 ]
  block[1].point[3] [ -0.078  0.077 ]
  block[1].z [0 0.21 ]

  # top
  block[2].points 4
  block[2].point[0] [ -0.02 -0.077 ]
  block[2].point[1] [  0.078 -0.077 ]
  block[2].point[2] [  0.078  0.077 ]
  block[2].point[3] [ -0.02  0.077 ]
  block[2].z [0.12 0.21 ]  

  # laser bit
  block[3].points 4
  block[3].point[0] [ -0.02 -0.05 ]
  block[3].point[1] [  0.06 -0.05 ]
  block[3].point[2] [  0.06  0.05 ]
  block[3].point[3] [ -0.02  0.05 ]
  block[3].z [0.02 0.12 ]  
  block[3].color "gray10"
)



