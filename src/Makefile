
##############################################################################
# This file should be called from the top-level make file, 
# which will set the values of the key directories


##############################################################################
# These are DEFAULT values -- they will get overridden by the top-level Makefile
# They are just here so you can compile stuff while testing new code.

VERSION=$(shell awk '{print substr($$3,2,length($$3)-2);}' ../VERSION)
#PLAYER_DIR = /usr/local/player$(VERSION)
PLAYER_DIR = ../../player


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
       usc_pioneer.o puck.o gripperdevice.o gpsdevice.o

# Flags common to all targets
#
CCFLAGS = -g3 -Wall -I../include -I$(PLAYER_INC) $(ZLIB_FLAGS)
#CCFLAGS = -O2 -Wall -I../include -I$(PLAYER_INC) $(ZLIB_FLAGS)
#LINKFLAGS = -lm -lstdc++ -lpthread  -L$(PLAYER_LIB) -lplayerclient $(ZLIB_LIBS)
LINKFLAGS = -lm -lstdc++ -lpthread  -L$(PLAYER_LIB) $(ZLIB_LIBS)

# Standard tools
#
CC := g++
LINKER = gcc
MAKEDEP = makedepend
#MAKEDEP = /home/gerkey/holding/Xbin/bin/makedepend

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
	$(MAKEDEP) -Y -I../include -I$(PLAYER_INC) -I$(RTK_INC) -s "# Dependancies (make dep)" *.cc 2>/dev/null

%.o : %.cc
	${CC} ${CCFLAGS} -c $< -o $@


##############################################################################
# Dependancies (make dep)

boxobstacle.o: ../include/world.hh ../include/image.h ../include/entity.hh
boxobstacle.o: ../include/stage_types.hh ../include/guiexport.hh
boxobstacle.o: ../../player/include/stage.h ../../player/include/messages.h
boxobstacle.o: ../../player/include/playercommon.h ../include/boxobstacle.hh
broadcastdevice.o: ../../player/include/stage.h
broadcastdevice.o: ../../player/include/messages.h
broadcastdevice.o: ../../player/include/playercommon.h ../include/world.hh
broadcastdevice.o: ../include/image.h ../include/entity.hh
broadcastdevice.o: ../include/stage_types.hh ../include/guiexport.hh
broadcastdevice.o: ../include/broadcastdevice.hh ../include/playerdevice.hh
entity.o: ../include/entity.hh ../include/stage_types.hh
entity.o: ../include/guiexport.hh ../../player/include/stage.h
entity.o: ../../player/include/messages.h ../../player/include/playercommon.h
entityfactory.o: ../include/entityfactory.hh ../include/boxobstacle.hh
entityfactory.o: ../include/entity.hh ../include/stage_types.hh
entityfactory.o: ../include/guiexport.hh ../../player/include/stage.h
entityfactory.o: ../../player/include/messages.h
entityfactory.o: ../../player/include/playercommon.h
entityfactory.o: ../include/laserbeacon.hh ../include/puck.hh
entityfactory.o: ../include/visionbeacon.hh ../include/usc_pioneer.hh
entityfactory.o: ../include/pioneermobiledevice.hh ../include/playerdevice.hh
entityfactory.o: ../include/laserdevice.hh ../include/laserbeacondevice.hh
entityfactory.o: ../include/playerserver.hh ../include/gripperdevice.hh
entityfactory.o: ../include/sonardevice.hh ../include/ptzdevice.hh
entityfactory.o: ../include/visiondevice.hh ../include/gpsdevice.hh
gpsdevice.o: ../../player/include/stage.h ../../player/include/messages.h
gpsdevice.o: ../../player/include/playercommon.h ../include/world.hh
gpsdevice.o: ../include/image.h ../include/entity.hh
gpsdevice.o: ../include/stage_types.hh ../include/guiexport.hh
gpsdevice.o: ../include/gpsdevice.hh ../include/playerdevice.hh
gripperdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
gripperdevice.o: ../include/stage_types.hh ../include/guiexport.hh
gripperdevice.o: ../../player/include/stage.h ../../player/include/messages.h
gripperdevice.o: ../../player/include/playercommon.h
gripperdevice.o: ../include/gripperdevice.hh ../include/playerdevice.hh
image.o: ../include/image.h
laserbeacon.o: ../include/world.hh ../include/image.h ../include/entity.hh
laserbeacon.o: ../include/stage_types.hh ../include/guiexport.hh
laserbeacon.o: ../../player/include/stage.h ../../player/include/messages.h
laserbeacon.o: ../../player/include/playercommon.h ../include/laserbeacon.hh
laserbeacondevice.o: ../../player/include/stage.h
laserbeacondevice.o: ../../player/include/messages.h
laserbeacondevice.o: ../../player/include/playercommon.h ../include/world.hh
laserbeacondevice.o: ../include/image.h ../include/entity.hh
laserbeacondevice.o: ../include/stage_types.hh ../include/guiexport.hh
laserbeacondevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh
laserdevice.o: ../../player/include/stage.h ../../player/include/messages.h
laserdevice.o: ../../player/include/playercommon.h ../include/world.hh
laserdevice.o: ../include/image.h ../include/entity.hh
laserdevice.o: ../include/stage_types.hh ../include/guiexport.hh
laserdevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
main.o: ../VERSION ../include/world.hh ../include/image.h
main.o: ../include/entity.hh ../include/stage_types.hh
main.o: ../include/guiexport.hh ../../player/include/stage.h
main.o: ../../player/include/messages.h ../../player/include/playercommon.h
miscdevice.o: ../../player/include/stage.h ../../player/include/messages.h
miscdevice.o: ../../player/include/playercommon.h ../include/world.hh
miscdevice.o: ../include/image.h ../include/entity.hh
miscdevice.o: ../include/stage_types.hh ../include/guiexport.hh
miscdevice.o: ../include/miscdevice.hh ../include/playerdevice.hh
pioneermobiledevice.o: ../include/world.hh ../include/image.h
pioneermobiledevice.o: ../include/entity.hh ../include/stage_types.hh
pioneermobiledevice.o: ../include/guiexport.hh ../../player/include/stage.h
pioneermobiledevice.o: ../../player/include/messages.h
pioneermobiledevice.o: ../../player/include/playercommon.h
pioneermobiledevice.o: ../include/pioneermobiledevice.hh
pioneermobiledevice.o: ../include/playerdevice.hh
playerdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
playerdevice.o: ../include/stage_types.hh ../include/guiexport.hh
playerdevice.o: ../../player/include/stage.h ../../player/include/messages.h
playerdevice.o: ../../player/include/playercommon.h
playerdevice.o: ../include/playerserver.hh ../include/playerdevice.hh
playerserver.o: ../../player/include/stage.h ../../player/include/messages.h
playerserver.o: ../../player/include/playercommon.h ../include/world.hh
playerserver.o: ../include/image.h ../include/entity.hh
playerserver.o: ../include/stage_types.hh ../include/guiexport.hh
playerserver.o: ../include/playerserver.hh
ptzdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
ptzdevice.o: ../include/stage_types.hh ../include/guiexport.hh
ptzdevice.o: ../../player/include/stage.h ../../player/include/messages.h
ptzdevice.o: ../../player/include/playercommon.h ../include/ptzdevice.hh
ptzdevice.o: ../include/playerdevice.hh
puck.o: ../include/world.hh ../include/image.h ../include/entity.hh
puck.o: ../include/stage_types.hh ../include/guiexport.hh
puck.o: ../../player/include/stage.h ../../player/include/messages.h
puck.o: ../../player/include/playercommon.h ../include/puck.hh
rtk_main.o: ../VERSION ../include/world.hh ../include/image.h
rtk_main.o: ../include/entity.hh ../include/stage_types.hh
rtk_main.o: ../include/guiexport.hh ../../player/include/stage.h
rtk_main.o: ../../player/include/messages.h
rtk_main.o: ../../player/include/playercommon.h ../rtk/include/rtk_ui.hh
rtk_main.o: ../rtk/include/rtk_types.hh ../rtk/include/rtk_msg.hh
rtk_main.o: ../rtk/include/rtk_msg_router.hh
rtk_main.o: ../rtk/include/rtk_ui_draw_data.hh ../rtk/include/rtk_ui_data.hh
rtk_main.o: ../rtk/include/rtk_string.hh ../rtk/include/rtk_ui_mouse_data.hh
rtk_main.o: ../rtk/include/rtk_ui_property_data.hh
rtk_main.o: ../rtk/include/rtk_ui_button_data.hh
rtk_main.o: ../rtk/include/rtk_ui_pane.hh ../rtk/include/rtk_ui_widget.hh
rtk_main.o: ../rtk/include/rtk_ui_app.hh ../rtk/include/rtk_agent.hh
rtk_main.o: ../rtk/include/rtk_cfg_file.hh
rtk_main.o: ../rtk/include/rtk_ui_scroll_view.hh
rtk_main.o: ../rtk/include/rtk_ui_property_view.hh
rtk_main.o: ../rtk/include/rtk_ui_button_view.hh
sonardevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
sonardevice.o: ../include/stage_types.hh ../include/guiexport.hh
sonardevice.o: ../../player/include/stage.h ../../player/include/messages.h
sonardevice.o: ../../player/include/playercommon.h ../include/sonardevice.hh
sonardevice.o: ../include/playerdevice.hh
usc_pioneer.o: ../include/world.hh ../include/image.h ../include/entity.hh
usc_pioneer.o: ../include/stage_types.hh ../include/guiexport.hh
usc_pioneer.o: ../../player/include/stage.h ../../player/include/messages.h
usc_pioneer.o: ../../player/include/playercommon.h ../include/usc_pioneer.hh
usc_pioneer.o: ../include/playerserver.hh ../include/pioneermobiledevice.hh
usc_pioneer.o: ../include/playerdevice.hh ../include/miscdevice.hh
usc_pioneer.o: ../include/sonardevice.hh ../include/laserdevice.hh
usc_pioneer.o: ../include/laserbeacondevice.hh ../include/ptzdevice.hh
usc_pioneer.o: ../include/visiondevice.hh
visionbeacon.o: ../include/world.hh ../include/image.h ../include/entity.hh
visionbeacon.o: ../include/stage_types.hh ../include/guiexport.hh
visionbeacon.o: ../../player/include/stage.h ../../player/include/messages.h
visionbeacon.o: ../../player/include/playercommon.h
visionbeacon.o: ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
visiondevice.o: ../include/stage_types.hh ../include/guiexport.hh
visiondevice.o: ../../player/include/stage.h ../../player/include/messages.h
visiondevice.o: ../../player/include/playercommon.h
visiondevice.o: ../include/visiondevice.hh ../include/playerdevice.hh
visiondevice.o: ../include/ptzdevice.hh
world.o: ../../player/include/stage.h ../../player/include/messages.h
world.o: ../../player/include/playercommon.h ../include/world.hh
world.o: ../include/image.h ../include/entity.hh ../include/stage_types.hh
world.o: ../include/guiexport.hh
world_load.o: ../include/stage_types.hh ../include/world.hh
world_load.o: ../include/image.h ../include/entity.hh ../include/guiexport.hh
world_load.o: ../../player/include/stage.h ../../player/include/messages.h
world_load.o: ../../player/include/playercommon.h ../include/entityfactory.hh
xgui.o: ../include/xgui.hh ../include/guiexport.hh
xgui.o: ../../player/include/stage.h ../../player/include/messages.h
xgui.o: ../../player/include/playercommon.h ../include/world.hh
xgui.o: ../include/image.h ../include/entity.hh ../include/stage_types.hh
xgui.o: ../include/pioneermobiledevice.hh ../include/playerdevice.hh
