# Common parameters to all pioneers
define pioneerBase position (
  color "red"			# Default color.
  
  drive "diff"		 	# Differential steering model.
  
  gui_nose 1                  	# Draw a nose on the robot so we can see which way it points

  obstacle_return 1           	# Can hit things.
  
  laser_return 0              	# Robot body seen by other lasers. If you want to model 
  											# real pioneers with SICK lasers, where the laser is 
											# taller than the robot.
  
  ranger_return 1             	# Seen by other sonar.
  
  blobfinder_return 1         	# Seen by other blobfinders.
  
  fiducial_return 2           	# Seen as "2" by other fiducial sensors.

#  localization "odom"         		# Change to "gps" to have impossibly perfect, global odometry
#  odom_error [ 0.05 0.05 0.1 ]   	# Odometry error or slip in X, Y and Theta
                                   	# (Uniform random distribution) 
  localization "gps"         	
  
  localization_origin [0 0 0] 	# Start odometry at (0, 0, 0).
  
)
