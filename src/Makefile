
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
# added src because lib doesn't get included in the distro
#	BPG
#RTK_LIB = $(RTK_DIR)/lib $(RTK_DIR)/src
RTK_LIB = $(RTK_DIR)/src


##############################################################################
# Build section

# Objects common to all targets
#
OBJS = world.o world_load.o image.o \
		entity.o entityfactory.o \
		playerserver.o playerdevice.o \
		pioneermobiledevice.o laserdevice.o \
		visiondevice.o ptzdevice.o sonardevice.o \
		laserbeacondevice.o broadcastdevice.o miscdevice.o \
		boxobstacle.o laserbeacon.o visionbeacon.o \
		usc_pioneer.o puck.o

# Flags common to all targets
#
#CCFLAGS = -g3 -Wall -I../include -I$(PLAYER_INC) $(ZLIB_FLAGS)
CCFLAGS = -O2 -Wall -I../include -I$(PLAYER_INC) $(ZLIB_FLAGS)
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

boxobstacle.o: ../include/world.hh ../include/image.h ../include/entity.hh
boxobstacle.o: ../include/stage_types.hh ../include/guiexport.hh
boxobstacle.o: /usr/local/player0.9.1/include/stage.h
boxobstacle.o: /usr/local/player0.9.1/include/messages.h
boxobstacle.o: /usr/local/player0.9.1/include/playercommon.h
boxobstacle.o: ../include/puck.hh ../include/boxobstacle.hh
broadcastdevice.o: /usr/local/player0.9.1/include/stage.h
broadcastdevice.o: /usr/local/player0.9.1/include/messages.h
broadcastdevice.o: /usr/local/player0.9.1/include/playercommon.h
broadcastdevice.o: ../include/world.hh ../include/image.h
broadcastdevice.o: ../include/entity.hh ../include/stage_types.hh
broadcastdevice.o: ../include/guiexport.hh ../include/puck.hh
broadcastdevice.o: ../include/broadcastdevice.hh ../include/playerdevice.hh
entity.o: ../include/entity.hh ../include/stage_types.hh
entity.o: ../include/guiexport.hh /usr/local/player0.9.1/include/stage.h
entity.o: /usr/local/player0.9.1/include/messages.h
entity.o: /usr/local/player0.9.1/include/playercommon.h
entityfactory.o: ../include/entityfactory.hh ../include/boxobstacle.hh
entityfactory.o: ../include/entity.hh ../include/stage_types.hh
entityfactory.o: ../include/guiexport.hh
entityfactory.o: /usr/local/player0.9.1/include/stage.h
entityfactory.o: /usr/local/player0.9.1/include/messages.h
entityfactory.o: /usr/local/player0.9.1/include/playercommon.h
entityfactory.o: ../include/laserbeacon.hh ../include/puck.hh
entityfactory.o: ../include/visionbeacon.hh ../include/usc_pioneer.hh
entityfactory.o: ../include/pioneermobiledevice.hh ../include/playerdevice.hh
entityfactory.o: ../include/laserdevice.hh ../include/playerserver.hh
image.o: ../include/image.h
laserbeacon.o: ../include/world.hh ../include/image.h ../include/entity.hh
laserbeacon.o: ../include/stage_types.hh ../include/guiexport.hh
laserbeacon.o: /usr/local/player0.9.1/include/stage.h
laserbeacon.o: /usr/local/player0.9.1/include/messages.h
laserbeacon.o: /usr/local/player0.9.1/include/playercommon.h
laserbeacon.o: ../include/puck.hh ../include/laserbeacon.hh
laserbeacondevice.o: /usr/local/player0.9.1/include/stage.h
laserbeacondevice.o: /usr/local/player0.9.1/include/messages.h
laserbeacondevice.o: /usr/local/player0.9.1/include/playercommon.h
laserbeacondevice.o: ../include/world.hh ../include/image.h
laserbeacondevice.o: ../include/entity.hh ../include/stage_types.hh
laserbeacondevice.o: ../include/guiexport.hh ../include/puck.hh
laserbeacondevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh
laserdevice.o: /usr/local/player0.9.1/include/stage.h
laserdevice.o: /usr/local/player0.9.1/include/messages.h
laserdevice.o: /usr/local/player0.9.1/include/playercommon.h
laserdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
laserdevice.o: ../include/stage_types.hh ../include/guiexport.hh
laserdevice.o: ../include/puck.hh ../include/laserdevice.hh
laserdevice.o: ../include/playerdevice.hh
main.o: ../VERSION ../include/world.hh ../include/image.h
main.o: ../include/entity.hh ../include/stage_types.hh
main.o: ../include/guiexport.hh /usr/local/player0.9.1/include/stage.h
main.o: /usr/local/player0.9.1/include/messages.h
main.o: /usr/local/player0.9.1/include/playercommon.h ../include/puck.hh
miscdevice.o: /usr/local/player0.9.1/include/stage.h
miscdevice.o: /usr/local/player0.9.1/include/messages.h
miscdevice.o: /usr/local/player0.9.1/include/playercommon.h
miscdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
miscdevice.o: ../include/stage_types.hh ../include/guiexport.hh
miscdevice.o: ../include/puck.hh ../include/miscdevice.hh
miscdevice.o: ../include/playerdevice.hh
pioneermobiledevice.o: ../include/world.hh ../include/image.h
pioneermobiledevice.o: ../include/entity.hh ../include/stage_types.hh
pioneermobiledevice.o: ../include/guiexport.hh
pioneermobiledevice.o: /usr/local/player0.9.1/include/stage.h
pioneermobiledevice.o: /usr/local/player0.9.1/include/messages.h
pioneermobiledevice.o: /usr/local/player0.9.1/include/playercommon.h
pioneermobiledevice.o: ../include/puck.hh ../include/pioneermobiledevice.hh
pioneermobiledevice.o: ../include/playerdevice.hh
playerdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
playerdevice.o: ../include/stage_types.hh ../include/guiexport.hh
playerdevice.o: /usr/local/player0.9.1/include/stage.h
playerdevice.o: /usr/local/player0.9.1/include/messages.h
playerdevice.o: /usr/local/player0.9.1/include/playercommon.h
playerdevice.o: ../include/puck.hh ../include/playerserver.hh
playerdevice.o: ../include/playerdevice.hh
playerserver.o: /usr/local/player0.9.1/include/stage.h
playerserver.o: /usr/local/player0.9.1/include/messages.h
playerserver.o: /usr/local/player0.9.1/include/playercommon.h
playerserver.o: ../include/world.hh ../include/image.h ../include/entity.hh
playerserver.o: ../include/stage_types.hh ../include/guiexport.hh
playerserver.o: ../include/puck.hh ../include/playerserver.hh
ptzdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
ptzdevice.o: ../include/stage_types.hh ../include/guiexport.hh
ptzdevice.o: /usr/local/player0.9.1/include/stage.h
ptzdevice.o: /usr/local/player0.9.1/include/messages.h
ptzdevice.o: /usr/local/player0.9.1/include/playercommon.h ../include/puck.hh
ptzdevice.o: ../include/ptzdevice.hh ../include/playerdevice.hh
puck.o: ../include/world.hh ../include/image.h ../include/entity.hh
puck.o: ../include/stage_types.hh ../include/guiexport.hh
puck.o: /usr/local/player0.9.1/include/stage.h
puck.o: /usr/local/player0.9.1/include/messages.h
puck.o: /usr/local/player0.9.1/include/playercommon.h ../include/puck.hh
rtk_main.o: ../VERSION ../include/world.hh ../include/image.h
rtk_main.o: ../include/entity.hh ../include/stage_types.hh
rtk_main.o: ../include/guiexport.hh /usr/local/player0.9.1/include/stage.h
rtk_main.o: /usr/local/player0.9.1/include/messages.h
rtk_main.o: /usr/local/player0.9.1/include/playercommon.h ../include/puck.hh
sonardevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
sonardevice.o: ../include/stage_types.hh ../include/guiexport.hh
sonardevice.o: /usr/local/player0.9.1/include/stage.h
sonardevice.o: /usr/local/player0.9.1/include/messages.h
sonardevice.o: /usr/local/player0.9.1/include/playercommon.h
sonardevice.o: ../include/puck.hh ../include/sonardevice.hh
sonardevice.o: ../include/playerdevice.hh
usc_pioneer.o: ../include/world.hh ../include/image.h ../include/entity.hh
usc_pioneer.o: ../include/stage_types.hh ../include/guiexport.hh
usc_pioneer.o: /usr/local/player0.9.1/include/stage.h
usc_pioneer.o: /usr/local/player0.9.1/include/messages.h
usc_pioneer.o: /usr/local/player0.9.1/include/playercommon.h
usc_pioneer.o: ../include/puck.hh ../include/usc_pioneer.hh
usc_pioneer.o: ../include/playerserver.hh ../include/pioneermobiledevice.hh
usc_pioneer.o: ../include/playerdevice.hh ../include/miscdevice.hh
usc_pioneer.o: ../include/sonardevice.hh ../include/laserdevice.hh
usc_pioneer.o: ../include/laserbeacondevice.hh ../include/ptzdevice.hh
usc_pioneer.o: ../include/visiondevice.hh
visionbeacon.o: ../include/world.hh ../include/image.h ../include/entity.hh
visionbeacon.o: ../include/stage_types.hh ../include/guiexport.hh
visionbeacon.o: /usr/local/player0.9.1/include/stage.h
visionbeacon.o: /usr/local/player0.9.1/include/messages.h
visionbeacon.o: /usr/local/player0.9.1/include/playercommon.h
visionbeacon.o: ../include/puck.hh ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
visiondevice.o: ../include/stage_types.hh ../include/guiexport.hh
visiondevice.o: /usr/local/player0.9.1/include/stage.h
visiondevice.o: /usr/local/player0.9.1/include/messages.h
visiondevice.o: /usr/local/player0.9.1/include/playercommon.h
visiondevice.o: ../include/puck.hh ../include/visiondevice.hh
visiondevice.o: ../include/playerdevice.hh ../include/ptzdevice.hh
world.o: /usr/local/player0.9.1/include/stage.h
world.o: /usr/local/player0.9.1/include/messages.h
world.o: /usr/local/player0.9.1/include/playercommon.h ../include/world.hh
world.o: ../include/image.h ../include/entity.hh ../include/stage_types.hh
world.o: ../include/guiexport.hh ../include/puck.hh
world_load.o: ../include/stage_types.hh ../include/world.hh
world_load.o: ../include/image.h ../include/entity.hh ../include/guiexport.hh
world_load.o: /usr/local/player0.9.1/include/stage.h
world_load.o: /usr/local/player0.9.1/include/messages.h
world_load.o: /usr/local/player0.9.1/include/playercommon.h
world_load.o: ../include/puck.hh ../include/entityfactory.hh
xgui.o: ../include/xgui.hh ../include/guiexport.hh
xgui.o: /usr/local/player0.9.1/include/stage.h
xgui.o: /usr/local/player0.9.1/include/messages.h
xgui.o: /usr/local/player0.9.1/include/playercommon.h ../include/world.hh
xgui.o: ../include/image.h ../include/entity.hh ../include/stage_types.hh
xgui.o: ../include/puck.hh
