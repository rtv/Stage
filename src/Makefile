#*************************************************************************
# src/Makefile - makes the sources
# RTV
# $Id: Makefile,v 1.9.2.2 2000-12-06 03:57:22 ahoward Exp $
# ************************************************************************/

.SUFFIXES: .cc .o


MAKEDEP = makedepend

CFLAGS = -I../include -I$(PLAYERSRC)/include -I/usr/lib/g++-include -I/usr/X11R6/include -O2 -g

OBS = main.o robot.o world.o win.o image.o server.o \
	device.o playerdevice.o pioneermobiledevice.o laserdevice.o \
	visiondevice.o ptzdevice.o sonardevice.o \
	object.o objectfactory.o laserbeaconobject.o
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


##############################################################################
# Use these targets to build with RTK

PLAYER_INC = $(PLAYERSRC)/include
RTK_INC = ../rtk/include
RTK_LIB = ../rtk/lib

RTK_OBJS = $(OBS) rtkmain.o rtksimagent.o
RTK_CCFLAGS = -DINCLUDE_RTK \
			-g3 -Wall \
			-I../include \
			-I$(PLAYER_INC) \
			-I$(RTK_INC) `gtk-config --cflags`
RTK_LINKFLAGS = -L$(PLAYER_LIB) -lplayerclient \
			-L$(RTK_LIB) -lrtk \
			-lm -lstdc++ -lpthread `gtk-config --libs gtk gthread`

%.o : %.cc
	g++ ${RTK_CCFLAGS} -c $< -o $@

rtk-stage:  $(RTK_OBJS) 
	g++ $(RTK_OBJS) $(RTK_LINKFLAGS) -o rtk-stage


# DO NOT DELETE.  FOLLOWING IS FROM MAKEDEPEND

device.o: ../include/world.hh ../include/robot.h ../include/image.h
device.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
device.o: ../include/device.hh
image.o: ../include/image.h ../include/world.hh ../include/robot.h
image.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
laserbeacondevice.o: ../include/world.hh ../include/robot.h
laserbeacondevice.o: ../include/image.h ../include/object.hh
laserbeacondevice.o: ../include/rtk-types.hh ../include/win.h
laserbeacondevice.o: ../include/laserbeacondevice.hh ../include/device.hh
laserbeaconobject.o: ../include/world.hh ../include/robot.h
laserbeaconobject.o: ../include/image.h ../include/object.hh
laserbeaconobject.o: ../include/rtk-types.hh ../include/win.h
laserbeaconobject.o: ../include/laserbeaconobject.hh
laserdevice.o: ../include/world.hh ../include/robot.h ../include/image.h
laserdevice.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
laserdevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserdevice.o: ../include/device.hh
main.o: ../include/world.hh ../include/robot.h ../include/image.h
main.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
object.o: ../include/object.hh ../include/rtk-types.hh
objectfactory.o: ../include/robot.h ../include/image.h ../include/object.hh
objectfactory.o: ../include/rtk-types.hh ../include/laserbeaconobject.hh
pioneermobiledevice.o: ../include/world.hh ../include/robot.h
pioneermobiledevice.o: ../include/image.h ../include/object.hh
pioneermobiledevice.o: ../include/rtk-types.hh ../include/win.h
pioneermobiledevice.o: ../include/pioneermobiledevice.hh
pioneermobiledevice.o: ../include/playerdevice.hh ../include/device.hh
playerdevice.o: ../include/world.hh ../include/robot.h ../include/image.h
playerdevice.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
playerdevice.o: ../include/playerdevice.hh ../include/device.hh
posreader.o: ../include/ports.h
ptzdevice.o: ../include/world.hh ../include/robot.h ../include/image.h
ptzdevice.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
ptzdevice.o: ../include/ptzdevice.hh ../include/playerdevice.hh
ptzdevice.o: ../include/device.hh
robot.o: ../include/world.hh ../include/robot.h ../include/image.h
robot.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
robot.o: ../include/ports.h ../include/pioneermobiledevice.hh
robot.o: ../include/playerdevice.hh ../include/device.hh
robot.o: ../include/sonardevice.h ../include/laserdevice.hh
robot.o: ../include/visiondevice.hh ../include/ptzdevice.hh
robot.o: ../include/laserbeacondevice.hh
rtkmain.o: ../include/world.hh ../include/robot.h ../include/image.h
rtkmain.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
rtkmain.o: rtksimagent.hh
rtksimagent.o: rtksimagent.hh ../include/world.hh ../include/robot.h
rtksimagent.o: ../include/image.h ../include/object.hh
rtksimagent.o: ../include/rtk-types.hh ../include/win.h
server.o: ../include/world.hh ../include/robot.h ../include/image.h
server.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
server.o: ../include/ports.h
sonardevice.o: ../include/world.hh ../include/robot.h ../include/image.h
sonardevice.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
sonardevice.o: ../include/sonardevice.h ../include/playerdevice.hh
sonardevice.o: ../include/device.hh
visiondevice.o: ../include/world.hh ../include/robot.h ../include/image.h
visiondevice.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
visiondevice.o: ../include/visiondevice.hh ../include/playerdevice.hh
visiondevice.o: ../include/device.hh ../include/ptzdevice.hh
win.o: ../include/win.h ../include/world.hh ../include/robot.h
win.o: ../include/image.h ../include/object.hh ../include/rtk-types.hh
world.o: ../include/world.hh ../include/robot.h ../include/image.h
world.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
