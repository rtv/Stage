
define sick_laser laser
(
  # factory settings for LMS200	
  range_min 0.0
  range_max 8.0
  fov 180.0
  samples 361

  #samples 90 # still useful but much faster to compute

  color "blue"
  size [ 0.156 0.155 0.15 ] # dimensions from LMS200 data sheet	
)
