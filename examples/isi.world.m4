include(robots.m4)

set environment_file = rink.pnm
set pixels_per_meter =  20
set laser_res = 2

#instantiate robots
isi_robot(5,6,0,6665)
isi_robot(4,6,0,6666)

create movable_object pose 2.98 3.42 0 channel 2 
create movable_object pose 3.40 3.47 0 channel 2 
create movable_object pose 3.30 2.98 0 channel 2 
create movable_object pose 2.52 3.00 0 channel 2 
create movable_object pose 2.45 3.37 0 channel 2 
create movable_object pose 3.25 2.62 0 channel 2 
create movable_object pose 2.85 2.98 0 channel 2 
create movable_object pose 2.88 2.67 0 channel 2 

create movable_object pose 2.47 2.72 0 channel 2 
