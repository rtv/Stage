#  args are (<x>,<y>,<th>,<port>)
define(`forager',`
# Robot Expansion
#   The following code was expanded from the macro call:
#       $0($*)

create position_device radius 0.2 pose $1 $2 $3 port $4 shape circle
{
  create player_device pose -0.13 0 0 size 0.05 0.05 port $4
  create sonar_device port $4
  create ptz_device port $4
  {
    create vision_device port $4
  }
  create gripper_device pose 0.16 0 0 consume true port $4
  create gps_device pose 0 0 0 port $4
}
')

#  args are (<x>,<y>,<th>,<port>)
define(`simplerobot',`
# Robot Expansion
#   The following code was expanded from the macro call:
#        $0($*)

create position_device pose $1 $2 $3 port $4 shape circle
{
  create player_device port $4
  create sonar_device port $4
}
')

#  args are (<x>,<y>,<th>,<port>)
define(`lasergpsrobot',`
# Robot Expansion
#   The following code was expanded from the macro call:
#        $0($*)

create position_device pose $1 $2 $3 port $4 shape circle
{
  create player_device
  create laser_device
  create gps_device
}
')


