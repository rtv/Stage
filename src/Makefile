#*************************************************************************
# src/Makefile - makes the sources
# RTV
# $Id: Makefile,v 1.9.2.17 2001-03-13 00:18:22 ahoward Exp $
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
RTK_SRC = /usr/local/rtk

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

rtk_stage:  $(RTK_OBJS) 
	g++ $(RTK_OBJS) $(RTK_LINKFLAGS) -o rtk_stage

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
boxobstacle.o: ../include/object.hh ../include/image.h
boxobstacle.o: ../include/boxobstacle.hh
broadcastdevice.o: ../include/world.hh ../include/playerrobot.hh
broadcastdevice.o: ../include/object.hh ../include/image.h
broadcastdevice.o: ../include/broadcastdevice.hh ../include/playerdevice.hh
image.o: ../include/image.h
laserbeacon.o: ../include/world.hh ../include/playerrobot.hh
laserbeacon.o: ../include/object.hh ../include/image.h
laserbeacon.o: ../include/laserbeacon.hh
laserbeacondevice.o: ../include/world.hh ../include/playerrobot.hh
laserbeacondevice.o: ../include/object.hh ../include/image.h
laserbeacondevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh
laserdevice.o: ../include/world.hh ../include/playerrobot.hh
laserdevice.o: ../include/object.hh ../include/image.h
laserdevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
main.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
main.o: ../include/image.h ../include/win.h
miscdevice.o: ../include/world.hh ../include/playerrobot.hh
miscdevice.o: ../include/object.hh ../include/image.h
miscdevice.o: ../include/miscdevice.hh ../include/playerdevice.hh
object.o: ../include/object.hh ../include/objectfactory.hh
objectfactory.o: ../include/objectfactory.hh ../include/boxobstacle.hh
objectfactory.o: ../include/object.hh ../include/laserbeacon.hh
objectfactory.o: ../include/visionbeacon.hh ../include/usc_pioneer.hh
pioneermobiledevice.o: ../include/world.hh ../include/playerrobot.hh
pioneermobiledevice.o: ../include/object.hh ../include/image.h
pioneermobiledevice.o: ../include/pioneermobiledevice.hh
pioneermobiledevice.o: ../include/playerdevice.hh
playerdevice.o: ../include/world.hh ../include/playerrobot.hh
playerdevice.o: ../include/object.hh ../include/image.h
playerdevice.o: ../include/playerdevice.hh
playerrobot.o: ../include/world.hh ../include/playerrobot.hh
playerrobot.o: ../include/object.hh ../include/image.h
posreader.o: ../include/ports.h
ptzdevice.o: ../include/world.hh ../include/playerrobot.hh
ptzdevice.o: ../include/object.hh ../include/image.h ../include/ptzdevice.hh
ptzdevice.o: ../include/playerdevice.hh
rtkmain.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
rtkmain.o: ../include/image.h
rtksimagent.o: ../include/world.hh ../include/playerrobot.hh
rtksimagent.o: ../include/object.hh ../include/image.h
rtksimagent.o: ../include/rtksimagent.hh
server.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
server.o: ../include/image.h ../include/ports.h
sonardevice.o: ../include/world.hh ../include/playerrobot.hh
sonardevice.o: ../include/object.hh ../include/image.h
sonardevice.o: ../include/sonardevice.hh ../include/playerdevice.hh
usc_pioneer.o: ../include/world.hh ../include/playerrobot.hh
usc_pioneer.o: ../include/object.hh ../include/image.h
usc_pioneer.o: ../include/usc_pioneer.hh ../include/pioneermobiledevice.hh
usc_pioneer.o: ../include/playerdevice.hh ../include/miscdevice.hh
usc_pioneer.o: ../include/sonardevice.hh ../include/laserdevice.hh
visionbeacon.o: ../include/world.hh ../include/playerrobot.hh
visionbeacon.o: ../include/object.hh ../include/image.h
visionbeacon.o: ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh ../include/playerrobot.hh
visiondevice.o: ../include/object.hh ../include/image.h
visiondevice.o: ../include/visiondevice.hh ../include/playerdevice.hh
visiondevice.o: ../include/ptzdevice.hh
win.o: ../include/win.h ../include/world.hh ../include/playerrobot.hh
win.o: ../include/object.hh ../include/image.h
world.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
world.o: ../include/image.h ../include/objectfactory.hh
