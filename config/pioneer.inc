
# Desc: Device definitions for Activemedia Pioneer robots.
# Author: Richard Vaughan, Andrew Howard
# Date: 10 Jun 2002
# CVS: $Id: pioneer.inc,v 1.1 2004-07-19 22:33:48 rtv Exp $

define pioneer2dx model
(
  size [0.44 0.33]

  nose 1
	
  color "red"

  mass 15.0  # Kg

  # body shape	
  lines.count 8
  lines.points[0] [0.23 0.05 0.15 0.15]
  lines.points[1] [0.15 0.15 -0.15 0.15]
  lines.points[2] [-0.15 0.15 -0.23 0.05]
  lines.points[3] [-0.23 0.05 -0.23 -0.05]
  lines.points[4] [-0.23 -0.05 -0.15 -0.15]
  lines.points[5] [-0.15 -0.15 0.15 -0.15]
  lines.points[6] [0.15 -0.15 0.23 -0.05]
  lines.points[7] [0.23 -0.05 0.23 0.05] 

  # sonar array
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

