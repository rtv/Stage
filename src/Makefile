#*************************************************************************
# src/Makefile - makes the sources
# RTV
# $Id: Makefile,v 1.8 2000-12-01 03:13:32 ahoward Exp $
# ************************************************************************/

.SUFFIXES: .cc .o


MAKEDEP = makedepend

CFLAGS = -I../include -I$(PLAYERSRC)/include -I/usr/lib/g++-include -I/usr/X11R6/include -O2 -g

OBS = main.o robot.o world.o win.o image.o server.o \
	device.o playerdevice.o pioneermobiledevice.o laserdevice.o visiondevice.o \
	sonardevice.o laserbeacondevice.o
LIBS = -L/usr/X11R6/lib -lX11 -lm -lpthread

clean:
	rm -f *.o *~ posreader stage

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
device.o: ../include/device.hh ../include/rtk-types.hh
image.o: ../include/image.h ../include/world.h ../include/robot.h
laserbeacondevice.o: ../include/world.h ../include/robot.h ../include/image.h
laserbeacondevice.o: ../include/laserbeacondevice.hh ../include/device.hh
laserbeacondevice.o: ../include/rtk-types.hh
laserdevice.o: ../include/world.h ../include/robot.h ../include/image.h
laserdevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserdevice.o: ../include/device.hh ../include/rtk-types.hh
main.o: ../include/world.h ../include/robot.h ../include/image.h
main.o: ../include/win.h
pioneermobiledevice.o: ../include/world.h ../include/robot.h
pioneermobiledevice.o: ../include/image.h ../include/pioneermobiledevice.h
playerdevice.o: ../include/world.h ../include/robot.h ../include/image.h
playerdevice.o: ../include/playerdevice.hh ../include/device.hh
playerdevice.o: ../include/rtk-types.hh
posreader.o: ../include/ports.h
robot.o: ../include/world.h ../include/robot.h ../include/image.h
robot.o: ../include/win.h ../include/ports.h ../include/laserdevice.hh
robot.o: ../include/playerdevice.hh ../include/device.hh
robot.o: ../include/rtk-types.hh ../include/laserbeacondevice.hh
server.o: ../include/world.h ../include/robot.h ../include/image.h
server.o: ../include/win.h ../include/ports.h
win.o: ../include/win.h ../include/world.h ../include/robot.h
win.o: ../include/image.h
world.o: ../include/world.h ../include/robot.h ../include/image.h
world.o: ../include/win.h
