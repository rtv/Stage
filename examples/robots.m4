#  args are (<x>,<y>,<th>,<port>)
define(`forager',`
# Robot Expansion
#   The following code was expanded from the macro call:
#       $0($*)

begin position_device
  pose ($1 $2 $3)
  port $4
  shape circle
  begin player_device 
    pose (-0.13 0 0)
  end
  begin sonar_device
  end
  begin ptz_device
    begin vision_device
    end
  end
  begin gripper_device
    pose (0.16 0 0) 
    consume true
  end
  begin gps_device
    pose (0 0 0)
  end
end
')

#  args are (<x>,<y>,<th>,<port>,<color>)
define(`simplerobot',`
# Robot Expansion
#   The following code was expanded from the macro call:
#        $0($*)

begin position_device 
  pose ($1 $2 $3)
  port $4 
  color $5
  shape circle
  begin player_device
  end
  begin sonar_device
  end
end
')

#  args are (<x>,<y>,<th>,<port>,<shape>,<color>)
define(`laserrobot',`
# Robot Expansion
#   The following code was expanded from the macro call:
#        $0($*)

begin position_device 
  pose ($1 $2 $3)
  port $4 
  shape $5
  color $6
  begin player_device
  end
  begin laser_device
  end
end
')


