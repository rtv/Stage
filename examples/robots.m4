#  args are (<x>,<y>,<th>,<port>)
define(`isi_robot',`
# Robot Expansion
#   The following code was expanded from the macro call:
#       $0($1,$2,$3,$4)
create player_device pose 0 0 0 port $4
create position_device pose $1 $2 $3 port $4 shape circle
{
  create sonar_device port $4
  create ptz_device port $4
  {
    create vision_device port $4
  }
  create gripper_device pose 0.16 0 0 consume false port $4
  create gps_device pose 0 0 0 port $4
}
')
