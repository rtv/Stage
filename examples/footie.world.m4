include(robots.m4)

set environment_file = rink.pnm
set pixels_per_meter = 80
set laser_res = 2

laserrobot(0.68,5.76,0,6665,circle,blue)
laserrobot(0.68,5.2,0,6666,circle,blue)
laserrobot(0.68,4.65,0,6667,circle,blue)
laserrobot(0.68,4.04,0,6668,circle,blue)

laserrobot(9.59,5.76,180,6669,circle,red)
laserrobot(9.59,5.2,180,6670,circle,red)
laserrobot(9.59,4.65,180,6671,circle,red)
laserrobot(9.59,4.04,180,6672,circle,red)

create movable_object pose 2.72334957055 2.72334957055 0 friction 0.035 radius 0.2 visible laserbright1
