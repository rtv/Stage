include(robots.m4)

set environment_file = rink.pnm
set pixels_per_meter =  20
set laser_res = 2

#instantiate robots
isi_robot(5,10,0,6665)
isi_robot(11,10,180,6666)
isi_robot(7,7,90,6667)

create movable_object pose 8.21 8.73 0 channel 2 
create movable_object pose 8.71 8.65 0 channel 2 
create movable_object pose 8.80 9.22 0 channel 2 
create movable_object pose 8.51 8.93 0 channel 2 
create movable_object pose 9.22 8.93 0 channel 2 
create movable_object pose 8.65 9.70 0 channel 2 
create movable_object pose 8.16 9.22 0 channel 2 
create movable_object pose 8.82 8.10 0 channel 2 
create movable_object pose 9.17 8.51 0 channel 2 

