include(robots.m4)
include(utils.m4)

set environment_file = rink.pnm
set pixels_per_meter = 10

enable truth_server
enable environment_server

# create a bunch of sonar robots
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+14),0,eval(i+6665))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+15),0,eval(i+6665+50))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+16),0,eval(i+6665+100))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+17),0,eval(i+6665+150))')
