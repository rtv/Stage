#*************************************************************************
# src/Makefile - makes the sources
# RTV
# $Id: Makefile,v 1.13 2001-02-06 01:27:06 gerkey Exp $
# ************************************************************************/

.SUFFIXES: .cc .o


MAKEDEP = makedepend

CFLAGS = -I../include -I$(PLAYERSRC)/include -I/usr/lib/g++-include -I/usr/X11R6/include -O3 -g

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
	$(MAKEDEP) -I../include -I$(PLAYERSRC)/include -Y -s "# DO NOT DELETE.  FOLLOWING IS FROM MAKEDEPEND" *.cc 2>/dev/null


posreader: posreader.cc
	g++ $(CFLAGS) -o posreader posreader.cc 

.cc.o: 
	g++ $(CFLAGS) -c $<

# DO NOT DELETE.  FOLLOWING IS FROM MAKEDEPEND

device.o: ../include/world.h ../include/robot.h
device.o: /home/gerkey/code/player/include/stage.h
device.o: /home/gerkey/code/player/include/messages.h
device.o: /home/gerkey/code/player/include/playercommon.h ../include/image.h
device.o: ../include/win.h ../include/device.hh
image.o: ../include/image.h ../include/world.h ../include/robot.h
image.o: /home/gerkey/code/player/include/stage.h
image.o: /home/gerkey/code/player/include/messages.h
image.o: /home/gerkey/code/player/include/playercommon.h ../include/win.h
laserbeacondevice.o: ../include/world.h ../include/robot.h
laserbeacondevice.o: /home/gerkey/code/player/include/stage.h
laserbeacondevice.o: /home/gerkey/code/player/include/messages.h
laserbeacondevice.o: /home/gerkey/code/player/include/playercommon.h
laserbeacondevice.o: ../include/image.h ../include/win.h
laserbeacondevice.o: ../include/laserbeacondevice.hh ../include/device.hh
laserdevice.o: ../include/world.h ../include/robot.h
laserdevice.o: /home/gerkey/code/player/include/stage.h
laserdevice.o: /home/gerkey/code/player/include/messages.h
laserdevice.o: /home/gerkey/code/player/include/playercommon.h
laserdevice.o: ../include/image.h ../include/win.h ../include/laserdevice.hh
laserdevice.o: ../include/playerdevice.hh ../include/device.hh
main.o: ../include/world.h ../include/robot.h
main.o: /home/gerkey/code/player/include/stage.h
main.o: /home/gerkey/code/player/include/messages.h
main.o: /home/gerkey/code/player/include/playercommon.h ../include/image.h
main.o: ../include/win.h
pioneermobiledevice.o: ../include/world.h ../include/robot.h
pioneermobiledevice.o: /home/gerkey/code/player/include/stage.h
pioneermobiledevice.o: /home/gerkey/code/player/include/messages.h
pioneermobiledevice.o: /home/gerkey/code/player/include/playercommon.h
pioneermobiledevice.o: ../include/image.h ../include/win.h
pioneermobiledevice.o: ../include/pioneermobiledevice.h
pioneermobiledevice.o: ../include/playerdevice.hh ../include/device.hh
playerdevice.o: ../include/world.h ../include/robot.h
playerdevice.o: /home/gerkey/code/player/include/stage.h
playerdevice.o: /home/gerkey/code/player/include/messages.h
playerdevice.o: /home/gerkey/code/player/include/playercommon.h
playerdevice.o: ../include/image.h ../include/win.h
playerdevice.o: ../include/playerdevice.hh ../include/device.hh
posreader.o: ../include/ports.h
ptzdevice.o: ../include/world.h ../include/robot.h
ptzdevice.o: /home/gerkey/code/player/include/stage.h
ptzdevice.o: /home/gerkey/code/player/include/messages.h
ptzdevice.o: /home/gerkey/code/player/include/playercommon.h
ptzdevice.o: ../include/image.h ../include/win.h ../include/ptzdevice.hh
ptzdevice.o: ../include/playerdevice.hh ../include/device.hh
robot.o: /home/gerkey/code/player/include/stage.h
robot.o: /home/gerkey/code/player/include/messages.h
robot.o: /home/gerkey/code/player/include/playercommon.h ../include/world.h
robot.o: ../include/robot.h ../include/image.h ../include/win.h
robot.o: ../include/ports.h ../include/pioneermobiledevice.h
robot.o: ../include/playerdevice.hh ../include/device.hh
robot.o: ../include/sonardevice.h ../include/laserdevice.hh
robot.o: ../include/visiondevice.hh ../include/ptzdevice.hh
robot.o: ../include/laserbeacondevice.hh
server.o: ../include/world.h ../include/robot.h
server.o: /home/gerkey/code/player/include/stage.h
server.o: /home/gerkey/code/player/include/messages.h
server.o: /home/gerkey/code/player/include/playercommon.h ../include/image.h
server.o: ../include/win.h ../include/ports.h
sonardevice.o: ../include/world.h ../include/robot.h
sonardevice.o: /home/gerkey/code/player/include/stage.h
sonardevice.o: /home/gerkey/code/player/include/messages.h
sonardevice.o: /home/gerkey/code/player/include/playercommon.h
sonardevice.o: ../include/image.h ../include/win.h ../include/sonardevice.h
sonardevice.o: ../include/playerdevice.hh ../include/device.hh
visiondevice.o: ../include/world.h ../include/robot.h
visiondevice.o: /home/gerkey/code/player/include/stage.h
visiondevice.o: /home/gerkey/code/player/include/messages.h
visiondevice.o: /home/gerkey/code/player/include/playercommon.h
visiondevice.o: ../include/image.h ../include/win.h
visiondevice.o: ../include/visiondevice.hh ../include/playerdevice.hh
visiondevice.o: ../include/device.hh ../include/ptzdevice.hh
win.o: ../include/win.h ../include/world.h ../include/robot.h
win.o: /home/gerkey/code/player/include/stage.h
win.o: /home/gerkey/code/player/include/messages.h
win.o: /home/gerkey/code/player/include/playercommon.h ../include/image.h
world.o: ../include/world.h ../include/robot.h
world.o: /home/gerkey/code/player/include/stage.h
world.o: /home/gerkey/code/player/include/messages.h
world.o: /home/gerkey/code/player/include/playercommon.h ../include/image.h
world.o: ../include/win.h
