
##############################################################################
# This file should be called from the top-level make file, 
# which will set the values of the key directories


##############################################################################
# These are DEFAULT values -- they will get overridden by the top-level Makefile
# They are just here so you can compile stuff while testing new code.

VERSION=$(shell awk '{print substr($$3,2,length($$3)-2);}' ../VERSION)
PLAYER_DIR = /usr/local/player$(VERSION)


##############################################################################
# Support for gzipped images; requires zlib.
# Comment out if you dont have/need zlib.

ZLIB_FLAGS = -DINCLUDE_ZLIB
ZLIB_LIBS = -lz


##############################################################################
# These are the key paths needed for the build

PLAYER_INC = $(PLAYER_DIR)/include
PLAYER_LIB = $(PLAYER_DIR)/lib
RTK_DIR = ../rtk
RTK_INC = $(RTK_DIR)/include
RTK_LIB = $(RTK_DIR)/lib


##############################################################################
# Build section

# Objects common to all targets
#
OBJS = world.o world_load.o image.o \
		object.o objectfactory.o \
		playerrobot.o playerdevice.o \
		pioneermobiledevice.o laserdevice.o \
		visiondevice.o ptzdevice.o sonardevice.o \
		laserbeacondevice.o broadcastdevice.o miscdevice.o \
		boxobstacle.o laserbeacon.o visionbeacon.o \
		usc_pioneer.o

# Flags common to all targets
#
CCFLAGS = -g3 -Wall -I../include -I$(PLAYER_INC) $(ZLIB_FLAGS)
LINKFLAGS = -lm -lstdc++ -lpthread  -L$(PLAYER_LIB) -lplayerclient $(ZLIB_LIBS)

# Standard tools
#
CC := g++
LINKER = gcc
MAKEDEP = makedepend

.SUFFIXES: .cc .o

##############################################################################
# Targets

# Target stage with no GUI
#
stage : OBJS += main.o
stage : $(OBJS) main.o 
	${LINKER} $(OBJS) $(LINKFLAGS) -o stage

# Target stage with Xlib GUI
#
xstage : CCFLAGS += -DINCLUDE_XGUI -I/usr/X11R6/include
xstage : OBJS += main.o xgui.o
xstage : $(OBJS) main.o xgui.o 
	${LINKER} $(OBJS) $(LINKFLAGS) -L/usr/X11R6/lib -lX11 -o xstage


# Target stage with RTK GUI
#
rtkstage : OBJS += rtk_main.o
rtkstage : CCFLAGS += -DINCLUDE_RTK -I$(RTK_INC) `gtk-config --cflags`
rtkstage : LINKFLAGS += -L$(RTK_LIB) -lrtk `gtk-config --libs gtk gthread`
rtkstage : $(OBJS) rtk_main.o 
	${LINKER} $(OBJS) $(LINKFLAGS) -o rtkstage

clean:
	rm -f *.o *~ core

dep:
	$(MAKEDEP) -Y -I../include -I$(PLAYER_INC) -s "# Dependancies (make dep)" *.cc 2>/dev/null

%.o : %.cc
	${CC} ${CCFLAGS} -c $< -o $@


##############################################################################
# Dependancies (make dep)

boxobstacle.o: ../include/world.hh ../include/playerrobot.hh
boxobstacle.o: ../include/object.hh ../include/stage_types.hh
boxobstacle.o: ../include/image.h ../include/boxobstacle.hh
broadcastdevice.o: /usr/local/player1.1.1beta/include/stage.h
broadcastdevice.o: /usr/local/player1.1.1beta/include/messages.h
broadcastdevice.o: /usr/local/player1.1.1beta/include/playercommon.h
broadcastdevice.o: ../include/world.hh ../include/playerrobot.hh
broadcastdevice.o: ../include/object.hh ../include/stage_types.hh
broadcastdevice.o: ../include/image.h ../include/broadcastdevice.hh
broadcastdevice.o: ../include/playerdevice.hh
image.o: ../include/image.h
laserbeacon.o: ../include/world.hh ../include/playerrobot.hh
laserbeacon.o: ../include/object.hh ../include/stage_types.hh
laserbeacon.o: ../include/image.h ../include/laserbeacon.hh
laserbeacondevice.o: /usr/local/player1.1.1beta/include/stage.h
laserbeacondevice.o: /usr/local/player1.1.1beta/include/messages.h
laserbeacondevice.o: /usr/local/player1.1.1beta/include/playercommon.h
laserbeacondevice.o: ../include/world.hh ../include/playerrobot.hh
laserbeacondevice.o: ../include/object.hh ../include/stage_types.hh
laserbeacondevice.o: ../include/image.h ../include/laserdevice.hh
laserbeacondevice.o: ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh
laserdevice.o: /usr/local/player1.1.1beta/include/stage.h
laserdevice.o: /usr/local/player1.1.1beta/include/messages.h
laserdevice.o: /usr/local/player1.1.1beta/include/playercommon.h
laserdevice.o: ../include/world.hh ../include/playerrobot.hh
laserdevice.o: ../include/object.hh ../include/stage_types.hh
laserdevice.o: ../include/image.h ../include/laserdevice.hh
laserdevice.o: ../include/playerdevice.hh
main.o: ../VERSION ../include/world.hh ../include/playerrobot.hh
main.o: ../include/object.hh ../include/stage_types.hh ../include/image.h
miscdevice.o: /usr/local/player1.1.1beta/include/stage.h
miscdevice.o: /usr/local/player1.1.1beta/include/messages.h
miscdevice.o: /usr/local/player1.1.1beta/include/playercommon.h
miscdevice.o: ../include/world.hh ../include/playerrobot.hh
miscdevice.o: ../include/object.hh ../include/stage_types.hh
miscdevice.o: ../include/image.h ../include/miscdevice.hh
miscdevice.o: ../include/playerdevice.hh
object.o: ../include/object.hh ../include/stage_types.hh
objectfactory.o: ../include/objectfactory.hh ../include/boxobstacle.hh
objectfactory.o: ../include/object.hh ../include/stage_types.hh
objectfactory.o: ../include/laserbeacon.hh ../include/visionbeacon.hh
objectfactory.o: ../include/usc_pioneer.hh
pioneermobiledevice.o: ../include/world.hh ../include/playerrobot.hh
pioneermobiledevice.o: ../include/object.hh ../include/stage_types.hh
pioneermobiledevice.o: ../include/image.h ../include/pioneermobiledevice.hh
pioneermobiledevice.o: /usr/local/player1.1.1beta/include/stage.h
pioneermobiledevice.o: /usr/local/player1.1.1beta/include/messages.h
pioneermobiledevice.o: /usr/local/player1.1.1beta/include/playercommon.h
pioneermobiledevice.o: ../include/playerdevice.hh
playerdevice.o: ../include/world.hh ../include/playerrobot.hh
playerdevice.o: ../include/object.hh ../include/stage_types.hh
playerdevice.o: ../include/image.h ../include/playerdevice.hh
playerdevice.o: /usr/local/player1.1.1beta/include/stage.h
playerdevice.o: /usr/local/player1.1.1beta/include/messages.h
playerdevice.o: /usr/local/player1.1.1beta/include/playercommon.h
playerrobot.o: /usr/local/player1.1.1beta/include/stage.h
playerrobot.o: /usr/local/player1.1.1beta/include/messages.h
playerrobot.o: /usr/local/player1.1.1beta/include/playercommon.h
playerrobot.o: ../include/world.hh ../include/playerrobot.hh
playerrobot.o: ../include/object.hh ../include/stage_types.hh
playerrobot.o: ../include/image.h
posreader.o: ../include/ports.h
ptzdevice.o: ../include/world.hh ../include/playerrobot.hh
ptzdevice.o: ../include/object.hh ../include/stage_types.hh
ptzdevice.o: ../include/image.h ../include/ptzdevice.hh
ptzdevice.o: ../include/playerdevice.hh
ptzdevice.o: /usr/local/player1.1.1beta/include/stage.h
ptzdevice.o: /usr/local/player1.1.1beta/include/messages.h
ptzdevice.o: /usr/local/player1.1.1beta/include/playercommon.h
rtk_main.o: ../VERSION ../include/world.hh ../include/playerrobot.hh
rtk_main.o: ../include/object.hh ../include/stage_types.hh ../include/image.h
server.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
server.o: ../include/stage_types.hh ../include/image.h ../include/ports.h
sonardevice.o: ../include/world.hh ../include/playerrobot.hh
sonardevice.o: ../include/object.hh ../include/stage_types.hh
sonardevice.o: ../include/image.h ../include/sonardevice.hh
sonardevice.o: ../include/playerdevice.hh
sonardevice.o: /usr/local/player1.1.1beta/include/stage.h
sonardevice.o: /usr/local/player1.1.1beta/include/messages.h
sonardevice.o: /usr/local/player1.1.1beta/include/playercommon.h
usc_pioneer.o: ../include/world.hh ../include/playerrobot.hh
usc_pioneer.o: ../include/object.hh ../include/stage_types.hh
usc_pioneer.o: ../include/image.h ../include/usc_pioneer.hh
usc_pioneer.o: ../include/pioneermobiledevice.hh
usc_pioneer.o: /usr/local/player1.1.1beta/include/stage.h
usc_pioneer.o: /usr/local/player1.1.1beta/include/messages.h
usc_pioneer.o: /usr/local/player1.1.1beta/include/playercommon.h
usc_pioneer.o: ../include/playerdevice.hh ../include/miscdevice.hh
usc_pioneer.o: ../include/sonardevice.hh ../include/laserdevice.hh
usc_pioneer.o: ../include/laserbeacondevice.hh
visionbeacon.o: ../include/world.hh ../include/playerrobot.hh
visionbeacon.o: ../include/object.hh ../include/stage_types.hh
visionbeacon.o: ../include/image.h ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh ../include/playerrobot.hh
visiondevice.o: ../include/object.hh ../include/stage_types.hh
visiondevice.o: ../include/image.h ../include/visiondevice.hh
visiondevice.o: ../include/playerdevice.hh
visiondevice.o: /usr/local/player1.1.1beta/include/stage.h
visiondevice.o: /usr/local/player1.1.1beta/include/messages.h
visiondevice.o: /usr/local/player1.1.1beta/include/playercommon.h
visiondevice.o: ../include/ptzdevice.hh
win.o: ../include/win.h ../include/world.hh ../include/playerrobot.hh
win.o: ../include/object.hh ../include/stage_types.hh ../include/image.h
world.o: /usr/local/player1.1.1beta/include/stage.h
world.o: /usr/local/player1.1.1beta/include/messages.h
world.o: /usr/local/player1.1.1beta/include/playercommon.h
world.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
world.o: ../include/stage_types.hh ../include/image.h
world_load.o: ../include/world.hh ../include/playerrobot.hh
world_load.o: ../include/object.hh ../include/stage_types.hh
world_load.o: ../include/image.h ../include/objectfactory.hh
