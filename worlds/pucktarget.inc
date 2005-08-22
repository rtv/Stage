
define pucktarget fiducialfinder
(
	range_min 0.0
	range_max 1.5
	fov 360.0
	samples 2
	size [0.25 0.25] 

	color "purple"
	#laser_return "bright"

	# cos(18) = 0.9510565	sin(18) = 0.309017
	# cos(54) = 0.587785	sin(54) = 0.809017
	polygons 1
	polygon[0].points 10
	polygon[0].point[0] [  0.238  0.077 ]
	polygon[0].point[1] [  0.1    0.1 ]
	polygon[0].point[2] [  0      0.250 ]
	polygon[0].point[3] [ -0.1    0.1 ]
	polygon[0].point[4] [ -0.238  0.077 ]
	polygon[0].point[5] [ -0.1   -0.1 ]
	polygon[0].point[6] [ -0.147 -0.202 ]
	polygon[0].point[7] [  0.0   -0.1 ]
	polygon[0].point[8] [  0.147 -0.202 ]
	polygon[0].point[9] [  0.1   -0.1 ]

)

