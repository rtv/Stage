include(robots.m4)
include(utils.m4)

set environment_file = rink.pnm
set pixels_per_meter = 10

enable truth_server
enable environment_server
#disable xs

define(`baseport', `40000')
create box 

# create a bunch of sonar robots
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+14),0,eval(i+baseport))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+15),0,eval(i+baseport+50))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+16),0,eval(i+baseport+100))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+17),0,eval(i+baseport+150))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+18),0,eval(i+baseport+200))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+20),0,eval(i+baseport+250))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+21),0,eval(i+baseport+300))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+22),0,eval(i+baseport+350))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+23),0,eval(i+baseport+400))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+24),0,eval(i+baseport+450))')

forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+25),0,eval(i+baseport+500))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+26),0,eval(i+baseport+550))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+27),0,eval(i+baseport+600))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+28),0,eval(i+baseport+650))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+29),0,eval(i+baseport+700))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+30),0,eval(i+baseport+750))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+31),0,eval(i+baseport+800))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+32),0,eval(i+baseport+850))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+33),0,eval(i+baseport+900))')
forloop(`i',0,49,`simplerobot(eval(i+14),eval(i+34),0,eval(i+baseport+950))')
