#*************************************************************************
# src/Makefile - makes the sources
# RTV
# $Id: Makefile,v 1.9.2.16 2001-03-07 01:57:43 ahoward Exp $
# ************************************************************************/

.SUFFIXES: .cc .o


MAKEDEP = makedepend

CFLAGS = -I../include -I$(PLAYERSRC)/include -I/usr/lib/g++-include -I/usr/X11R6/include -O2

OBS = world.o image.o \
		object.o objectfactory.o \
		playerrobot.o playerdevice.o \
		pioneermobiledevice.o laserdevice.o \
		visiondevice.o ptzdevice.o sonardevice.o \
		laserbeacondevice.o broadcastdevice.o miscdevice.o \
		boxobstacle.o laserbeacon.o visionbeacon.o \
		usc_pioneer.o
WIN_OBS = main.o win.o server.o
LIBS = -L/usr/X11R6/lib -lX11 -lm -lpthread


##############################################################################
# Use these targets to build with RTK

PLAYER_SRC = /usr/local/player
#RTK_SRC = /usr/local/rtk
RTK_SRC = ../../rtk

PLAYER_INC = $(PLAYER_SRC)/include
RTK_INC = $(RTK_SRC)/include
RTK_LIB = $(RTK_SRC)/lib

RTK_OBJS = $(OBS) rtkmain.o
RTK_CCFLAGS = 	-DINCLUDE_RTK \
				-g3 -Wall \
				-I../include \
				-I$(PLAYER_INC) \
				-I$(RTK_INC) `gtk-config --cflags`
RTK_LINKFLAGS = -L$(PLAYER_LIB) -lplayerclient \
				-L$(RTK_LIB) -lrtk \
				-lm -lstdc++ -lpthread `gtk-config --libs gtk gthread`

rtk-stage:  $(RTK_OBJS) 
	g++ $(RTK_OBJS) $(RTK_LINKFLAGS) -o rtk-stage

clean:
	rm -f *.o *~ rtk-stage

all: rtk-stage

dep:
	$(MAKEDEP) -I../include -Y -s "# DO NOT DELETE.  FOLLOWING IS FROM MAKEDEPEND" *.cc 2>/dev/null

.cc.o: 
	g++ $(CFLAGS) -c $<

%.o : %.cc
	g++ ${RTK_CCFLAGS} -c $< -o $@


# DO NOT DELETE.  FOLLOWING IS FROM MAKEDEPEND

boxobstacle.o: ../include/world.hh ../include/playerrobot.hh
boxobstacle.o: ../include/object.hh ../include/rtk-types.hh
boxobstacle.o: ../include/image.h ../include/boxobstacle.hh
broadcastdevice.o: ../include/world.hh ../include/playerrobot.hh
broadcastdevice.o: ../include/object.hh ../include/rtk-types.hh
broadcastdevice.o: ../include/image.h ../include/broadcastdevice.hh
broadcastdevice.o: ../include/playerdevice.hh
image.o: ../include/image.h
laserbeacon.o: ../include/world.hh ../include/playerrobot.hh
laserbeacon.o: ../include/object.hh ../include/rtk-types.hh
laserbeacon.o: ../include/image.h ../include/laserbeacon.hh
laserbeacondevice.o: ../include/world.hh ../include/playerrobot.hh
laserbeacondevice.o: ../include/object.hh ../include/rtk-types.hh
laserbeacondevice.o: ../include/image.h ../include/laserdevice.hh
laserbeacondevice.o: ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh
laserdevice.o: ../include/world.hh ../include/playerrobot.hh
laserdevice.o: ../include/object.hh ../include/rtk-types.hh
laserdevice.o: ../include/image.h ../include/laserdevice.hh
laserdevice.o: ../include/playerdevice.hh
main.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
main.o: ../include/rtk-types.hh ../include/image.h ../include/win.h
miscdevice.o: ../include/world.hh ../include/playerrobot.hh
miscdevice.o: ../include/object.hh ../include/rtk-types.hh ../include/image.h
miscdevice.o: ../include/miscdevice.hh ../include/playerdevice.hh
object.o: ../include/object.hh ../include/rtk-types.hh
object.o: ../include/objectfactory.hh
objectfactory.o: ../include/objectfactory.hh ../include/boxobstacle.hh
objectfactory.o: ../include/object.hh ../include/rtk-types.hh
objectfactory.o: ../include/laserbeacon.hh ../include/visionbeacon.hh
objectfactory.o: ../include/usc_pioneer.hh ../include/playerrobot.hh
objectfactory.o: ../include/pioneermobiledevice.hh ../include/playerdevice.hh
objectfactory.o: ../include/miscdevice.hh ../include/sonardevice.hh
objectfactory.o: ../include/laserdevice.hh ../include/ptzdevice.hh
objectfactory.o: ../include/visiondevice.hh ../include/laserbeacondevice.hh
objectfactory.o: ../include/broadcastdevice.hh
pioneermobiledevice.o: ../include/world.hh ../include/playerrobot.hh
pioneermobiledevice.o: ../include/object.hh ../include/rtk-types.hh
pioneermobiledevice.o: ../include/image.h ../include/pioneermobiledevice.hh
pioneermobiledevice.o: ../include/playerdevice.hh
playerdevice.o: ../include/world.hh ../include/playerrobot.hh
playerdevice.o: ../include/object.hh ../include/rtk-types.hh
playerdevice.o: ../include/image.h ../include/playerdevice.hh
playerrobot.o: ../include/world.hh ../include/playerrobot.hh
playerrobot.o: ../include/object.hh ../include/rtk-types.hh
playerrobot.o: ../include/image.h
posreader.o: ../include/ports.h
ptzdevice.o: ../include/world.hh ../include/playerrobot.hh
ptzdevice.o: ../include/object.hh ../include/rtk-types.hh ../include/image.h
ptzdevice.o: ../include/ptzdevice.hh ../include/playerdevice.hh
rtkmain.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
rtkmain.o: ../include/rtk-types.hh ../include/image.h
rtksimagent.o: ../include/world.hh ../include/playerrobot.hh
rtksimagent.o: ../include/object.hh ../include/rtk-types.hh
rtksimagent.o: ../include/image.h ../include/rtksimagent.hh
server.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
server.o: ../include/rtk-types.hh ../include/image.h ../include/ports.h
sonardevice.o: ../include/world.hh ../include/playerrobot.hh
sonardevice.o: ../include/object.hh ../include/rtk-types.hh
sonardevice.o: ../include/image.h ../include/sonardevice.hh
sonardevice.o: ../include/playerdevice.hh
usc_pioneer.o: ../include/world.hh ../include/playerrobot.hh
usc_pioneer.o: ../include/object.hh ../include/rtk-types.hh
usc_pioneer.o: ../include/image.h ../include/usc_pioneer.hh
usc_pioneer.o: ../include/pioneermobiledevice.hh ../include/playerdevice.hh
visionbeacon.o: ../include/world.hh ../include/playerrobot.hh
visionbeacon.o: ../include/object.hh ../include/rtk-types.hh
visionbeacon.o: ../include/image.h ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh ../include/playerrobot.hh
visiondevice.o: ../include/object.hh ../include/rtk-types.hh
visiondevice.o: ../include/image.h ../include/visiondevice.hh
visiondevice.o: ../include/playerdevice.hh ../include/ptzdevice.hh
win.o: ../include/win.h ../include/world.hh ../include/playerrobot.hh
win.o: ../include/object.hh ../include/rtk-types.hh ../include/image.h
world.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
world.o: ../include/rtk-types.hh ../include/image.h
world.o: ../include/objectfactory.hh
