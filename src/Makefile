#*************************************************************************
# src/Makefile - makes the sources
# RTV
# $Id: Makefile,v 1.9.2.5 2000-12-07 00:30:00 ahoward Exp $
# ************************************************************************/

.SUFFIXES: .cc .o


MAKEDEP = makedepend

CFLAGS = -I../include -I$(PLAYERSRC)/include -I/usr/lib/g++-include -I/usr/X11R6/include -O2 -g

OBS = main.o world.o image.o \
		object.o objectfactory.o \
		playerrobot.o playerdevice.o \
		pioneermobiledevice.o laserdevice.o \
		visiondevice.o ptzdevice.o sonardevice.o \
		laserbeaconobject.o visionbeacon.o
WIN_OBS = win.o server.o
LIBS = -L/usr/X11R6/lib -lX11 -lm -lpthread

clean:
	rm -f *.o *~ posreader stage

all: stage posreader

stage:  $(OBS) $(WIN_OBS)
	g++ $(LIBS) $(OBS) $(WIN_OBS) -o stage

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

image.o: ../include/image.h ../include/world.hh ../include/playerrobot.hh
image.o: ../include/object.hh ../include/rtk-types.hh ../include/win.h
laserbeacondevice.o: ../include/world.hh ../include/playerrobot.hh
laserbeacondevice.o: ../include/object.hh ../include/rtk-types.hh
laserbeacondevice.o: ../include/image.h ../include/win.h
laserbeacondevice.o: ../include/laserbeacondevice.hh
laserbeaconobject.o: ../include/world.hh ../include/playerrobot.hh
laserbeaconobject.o: ../include/object.hh ../include/rtk-types.hh
laserbeaconobject.o: ../include/image.h ../include/win.h
laserbeaconobject.o: ../include/laserbeaconobject.hh
laserdevice.o: ../include/world.hh ../include/playerrobot.hh
laserdevice.o: ../include/object.hh ../include/rtk-types.hh
laserdevice.o: ../include/image.h ../include/win.h ../include/laserdevice.hh
laserdevice.o: ../include/playerdevice.hh
main.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
main.o: ../include/rtk-types.hh ../include/image.h ../include/win.h
object.o: ../include/object.hh ../include/rtk-types.hh
object.o: ../include/objectfactory.hh
objectfactory.o: ../include/objectfactory.hh ../include/laserbeaconobject.hh
objectfactory.o: ../include/object.hh ../include/rtk-types.hh
objectfactory.o: ../include/visionbeacon.hh ../include/playerrobot.hh
objectfactory.o: ../include/pioneermobiledevice.hh ../include/playerdevice.hh
objectfactory.o: ../include/win.h ../include/world.hh ../include/image.h
objectfactory.o: ../include/laserdevice.hh
pioneermobiledevice.o: ../include/world.hh ../include/playerrobot.hh
pioneermobiledevice.o: ../include/object.hh ../include/rtk-types.hh
pioneermobiledevice.o: ../include/image.h ../include/win.h
pioneermobiledevice.o: ../include/pioneermobiledevice.hh
pioneermobiledevice.o: ../include/playerdevice.hh
playerdevice.o: ../include/world.hh ../include/playerrobot.hh
playerdevice.o: ../include/object.hh ../include/rtk-types.hh
playerdevice.o: ../include/image.h ../include/win.h
playerdevice.o: ../include/playerdevice.hh
playerrobot.o: ../include/world.hh ../include/playerrobot.hh
playerrobot.o: ../include/object.hh ../include/rtk-types.hh
playerrobot.o: ../include/image.h ../include/win.h
posreader.o: ../include/ports.h
ptzdevice.o: ../include/world.hh ../include/playerrobot.hh
ptzdevice.o: ../include/object.hh ../include/rtk-types.hh ../include/image.h
ptzdevice.o: ../include/win.h ../include/ptzdevice.hh
ptzdevice.o: ../include/playerdevice.hh
rtkmain.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
rtkmain.o: ../include/rtk-types.hh ../include/image.h ../include/win.h
rtkmain.o: ../include/rtksimagent.hh
rtksimagent.o: ../include/rtksimagent.hh ../include/world.hh
rtksimagent.o: ../include/playerrobot.hh ../include/object.hh
rtksimagent.o: ../include/rtk-types.hh ../include/image.h ../include/win.h
server.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
server.o: ../include/rtk-types.hh ../include/image.h ../include/win.h
server.o: ../include/ports.h
sonardevice.o: ../include/world.hh ../include/playerrobot.hh
sonardevice.o: ../include/object.hh ../include/rtk-types.hh
sonardevice.o: ../include/image.h ../include/win.h ../include/sonardevice.h
sonardevice.o: ../include/playerdevice.hh
visionbeacon.o: ../include/world.hh ../include/playerrobot.hh
visionbeacon.o: ../include/object.hh ../include/rtk-types.hh
visionbeacon.o: ../include/image.h ../include/win.h
visionbeacon.o: ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh ../include/playerrobot.hh
visiondevice.o: ../include/object.hh ../include/rtk-types.hh
visiondevice.o: ../include/image.h ../include/win.h
visiondevice.o: ../include/visiondevice.hh ../include/playerdevice.hh
visiondevice.o: ../include/ptzdevice.hh
win.o: ../include/win.h ../include/world.hh ../include/playerrobot.hh
win.o: ../include/object.hh ../include/rtk-types.hh ../include/image.h
world.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
world.o: ../include/rtk-types.hh ../include/image.h ../include/win.h
