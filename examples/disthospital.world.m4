# this is a simple for loop. use it like:
#   forloop(`i',1,10,`i ')
define(`forloop',
        `pushdef(`$1', `$2')_forloop(`$1', `$2', `$3', `$4')popdef(`$1')')
define(`_forloop',
        `$4`'ifelse($1, `$3', , 
        `define(`$1', incr($1))_forloop(`$1', `$2', `$3', `$4')')')

#  args are (<x>,<y>,<th>,<port>)
define(`simplerobot',`
# Robot Expansion
#   The following code was expanded from the macro call:
#        $0($*)

create position_device pose $1 $2 $3 port $4 shape circle
{
  create player_device port $4
  create sonar_device port $4
  create gps_device port $4
}
')
set environment_file = filled_hospital.pnm
set pixels_per_meter = 12

define(`baseport', `40000')

forloop(`i',0,299,`simplerobot(14,14,0,eval(i+baseport))')

