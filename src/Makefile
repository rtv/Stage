#*************************************************************************
# src/Makefile - makes the sources
# RTV
# $Id: Makefile,v 1.10 2000-12-06 02:31:51 vaughan Exp $
# ************************************************************************/

.SUFFIXES: .cc .o


MAKEDEP = makedepend

CFLAGS = -I../include -I$(PLAYERSRC)/include -I/usr/lib/g++-include -I/usr/X11R6/include -O2 -g

OBS = main.o robot.o world.o win.o image.o server.o \
	device.o playerdevice.o pioneermobiledevice.o laserdevice.o \
	visiondevice.o ptzdevice.o \
	sonardevice.o laserbeacondevice.o
LIBS = -L/usr/X11R6/lib -lX11 -lm -lpthread

clean:
	rm -f *.o *~ *.bak posreader stage

all: stage posreader

stage:  $(OBS) 
	g++ $(LIBS) $(OBS) -o stage

dep:
	$(MAKEDEP) -I../include -Y -s "# DO NOT DELETE.  FOLLOWING IS FROM MAKEDEPEND" *.cc 2>/dev/null


posreader: posreader.cc
	g++ $(CFLAGS) -o posreader posreader.cc 

.cc.o: 
	g++ $(CFLAGS) -c $<

# DO NOT DELETE.  FOLLOWING IS FROM MAKEDEPEND

device.o: ../include/world.h ../include/robot.h ../include/image.h
device.o: ../include/win.h ../include/device.hh ../include/rtk-types.hh
image.o: ../include/image.h ../include/world.h ../include/robot.h
image.o: ../include/win.h
laserbeacondevice.o: ../include/world.h ../include/robot.h ../include/image.h
laserbeacondevice.o: ../include/win.h ../include/laserbeacondevice.hh
laserbeacondevice.o: ../include/device.hh ../include/rtk-types.hh
laserdevice.o: ../include/world.h ../include/robot.h ../include/image.h
laserdevice.o: ../include/win.h ../include/laserdevice.hh
laserdevice.o: ../include/playerdevice.hh ../include/device.hh
laserdevice.o: ../include/rtk-types.hh
main.o: ../include/world.h ../include/robot.h ../include/image.h
main.o: ../include/win.h
pioneermobiledevice.o: ../include/world.h ../include/robot.h
pioneermobiledevice.o: ../include/image.h ../include/win.h
pioneermobiledevice.o: ../include/pioneermobiledevice.h
pioneermobiledevice.o: ../include/playerdevice.hh ../include/device.hh
pioneermobiledevice.o: ../include/rtk-types.hh
playerdevice.o: ../include/world.h ../include/robot.h ../include/image.h
playerdevice.o: ../include/win.h ../include/playerdevice.hh
playerdevice.o: ../include/device.hh ../include/rtk-types.hh
posreader.o: ../include/ports.h
ptzdevice.o: ../include/world.h ../include/robot.h ../include/image.h
ptzdevice.o: ../include/win.h ../include/ptzdevice.hh
ptzdevice.o: ../include/playerdevice.hh ../include/device.hh
ptzdevice.o: ../include/rtk-types.hh ../include/visiondevice.hh
robot.o: ../include/world.h ../include/robot.h ../include/image.h
robot.o: ../include/win.h ../include/ports.h ../include/pioneermobiledevice.h
robot.o: ../include/playerdevice.hh ../include/device.hh
robot.o: ../include/rtk-types.hh ../include/sonardevice.h
robot.o: ../include/laserdevice.hh ../include/visiondevice.hh
robot.o: ../include/ptzdevice.hh ../include/laserbeacondevice.hh
server.o: ../include/world.h ../include/robot.h ../include/image.h
server.o: ../include/win.h ../include/ports.h
sonardevice.o: ../include/world.h ../include/robot.h ../include/image.h
sonardevice.o: ../include/win.h ../include/sonardevice.h
sonardevice.o: ../include/playerdevice.hh ../include/device.hh
sonardevice.o: ../include/rtk-types.hh
visiondevice.o: ../include/world.h ../include/robot.h ../include/image.h
visiondevice.o: ../include/win.h ../include/visiondevice.hh
visiondevice.o: ../include/playerdevice.hh ../include/device.hh
visiondevice.o: ../include/rtk-types.hh
win.o: ../include/win.h ../include/world.h ../include/robot.h
win.o: ../include/image.h
world.o: ../include/world.h ../include/robot.h ../include/image.h
world.o: ../include/win.h
