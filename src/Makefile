
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
CCFLAGS = -g3 -Wall -I../include -I$(PLAYER_INC) $(ZLIB_FLAGS)
#CCFLAGS = -O2 -Wall -I../include -I$(PLAYER_INC) $(ZLIB_FLAGS)
LINKFLAGS = -lm -lstdc++ -lpthread  -L$(PLAYER_LIB) -lplayerclient $(ZLIB_LIBS)

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
boxobstacle.o: /usr/local/player0.9.1-Bleeding/include/stage.h
boxobstacle.o: /usr/local/player0.9.1-Bleeding/include/messages.h
boxobstacle.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
boxobstacle.o: ../include/boxobstacle.hh
broadcastdevice.o: /usr/local/player0.9.1-Bleeding/include/stage.h
broadcastdevice.o: /usr/local/player0.9.1-Bleeding/include/messages.h
broadcastdevice.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
broadcastdevice.o: ../include/world.hh ../include/image.h
broadcastdevice.o: ../include/entity.hh ../include/stage_types.hh
broadcastdevice.o: ../include/guiexport.hh ../include/broadcastdevice.hh
broadcastdevice.o: ../include/playerdevice.hh
entity.o: ../include/entity.hh ../include/stage_types.hh
entity.o: ../include/guiexport.hh
entity.o: /usr/local/player0.9.1-Bleeding/include/stage.h
entity.o: /usr/local/player0.9.1-Bleeding/include/messages.h
entity.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
entityfactory.o: ../include/entityfactory.hh ../include/boxobstacle.hh
entityfactory.o: ../include/entity.hh ../include/stage_types.hh
entityfactory.o: ../include/guiexport.hh
entityfactory.o: /usr/local/player0.9.1-Bleeding/include/stage.h
entityfactory.o: /usr/local/player0.9.1-Bleeding/include/messages.h
entityfactory.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
entityfactory.o: ../include/laserbeacon.hh ../include/puck.hh
entityfactory.o: ../include/visionbeacon.hh ../include/usc_pioneer.hh
entityfactory.o: ../include/pioneermobiledevice.hh ../include/playerdevice.hh
entityfactory.o: ../include/laserdevice.hh ../include/laserbeacondevice.hh
entityfactory.o: ../include/playerserver.hh
image.o: ../include/image.h
laserbeacon.o: ../include/world.hh ../include/image.h ../include/entity.hh
laserbeacon.o: ../include/stage_types.hh ../include/guiexport.hh
laserbeacon.o: /usr/local/player0.9.1-Bleeding/include/stage.h
laserbeacon.o: /usr/local/player0.9.1-Bleeding/include/messages.h
laserbeacon.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
laserbeacon.o: ../include/laserbeacon.hh
laserbeacondevice.o: /usr/local/player0.9.1-Bleeding/include/stage.h
laserbeacondevice.o: /usr/local/player0.9.1-Bleeding/include/messages.h
laserbeacondevice.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
laserbeacondevice.o: ../include/world.hh ../include/image.h
laserbeacondevice.o: ../include/entity.hh ../include/stage_types.hh
laserbeacondevice.o: ../include/guiexport.hh ../include/laserdevice.hh
laserbeacondevice.o: ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh
laserdevice.o: /usr/local/player0.9.1-Bleeding/include/stage.h
laserdevice.o: /usr/local/player0.9.1-Bleeding/include/messages.h
laserdevice.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
laserdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
laserdevice.o: ../include/stage_types.hh ../include/guiexport.hh
laserdevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
main.o: ../VERSION ../include/world.hh ../include/image.h
main.o: ../include/entity.hh ../include/stage_types.hh
main.o: ../include/guiexport.hh
main.o: /usr/local/player0.9.1-Bleeding/include/stage.h
main.o: /usr/local/player0.9.1-Bleeding/include/messages.h
main.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
miscdevice.o: /usr/local/player0.9.1-Bleeding/include/stage.h
miscdevice.o: /usr/local/player0.9.1-Bleeding/include/messages.h
miscdevice.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
miscdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
miscdevice.o: ../include/stage_types.hh ../include/guiexport.hh
miscdevice.o: ../include/miscdevice.hh ../include/playerdevice.hh
pioneermobiledevice.o: ../include/world.hh ../include/image.h
pioneermobiledevice.o: ../include/entity.hh ../include/stage_types.hh
pioneermobiledevice.o: ../include/guiexport.hh
pioneermobiledevice.o: /usr/local/player0.9.1-Bleeding/include/stage.h
pioneermobiledevice.o: /usr/local/player0.9.1-Bleeding/include/messages.h
pioneermobiledevice.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
pioneermobiledevice.o: ../include/pioneermobiledevice.hh
pioneermobiledevice.o: ../include/playerdevice.hh
playerdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
playerdevice.o: ../include/stage_types.hh ../include/guiexport.hh
playerdevice.o: /usr/local/player0.9.1-Bleeding/include/stage.h
playerdevice.o: /usr/local/player0.9.1-Bleeding/include/messages.h
playerdevice.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
playerdevice.o: ../include/playerserver.hh ../include/playerdevice.hh
playerserver.o: /usr/local/player0.9.1-Bleeding/include/stage.h
playerserver.o: /usr/local/player0.9.1-Bleeding/include/messages.h
playerserver.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
playerserver.o: ../include/world.hh ../include/image.h ../include/entity.hh
playerserver.o: ../include/stage_types.hh ../include/guiexport.hh
playerserver.o: ../include/playerserver.hh
ptzdevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
ptzdevice.o: ../include/stage_types.hh ../include/guiexport.hh
ptzdevice.o: /usr/local/player0.9.1-Bleeding/include/stage.h
ptzdevice.o: /usr/local/player0.9.1-Bleeding/include/messages.h
ptzdevice.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
ptzdevice.o: ../include/ptzdevice.hh ../include/playerdevice.hh
puck.o: ../include/world.hh ../include/image.h ../include/entity.hh
puck.o: ../include/stage_types.hh ../include/guiexport.hh
puck.o: /usr/local/player0.9.1-Bleeding/include/stage.h
puck.o: /usr/local/player0.9.1-Bleeding/include/messages.h
puck.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
puck.o: ../include/puck.hh
rtk_main.o: ../VERSION ../include/world.hh ../include/image.h
rtk_main.o: ../include/entity.hh ../include/stage_types.hh
rtk_main.o: ../include/guiexport.hh
rtk_main.o: /usr/local/player0.9.1-Bleeding/include/stage.h
rtk_main.o: /usr/local/player0.9.1-Bleeding/include/messages.h
rtk_main.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
rtk_main.o: ../rtk/include/rtk_ui.hh ../rtk/include/rtk_types.hh
rtk_main.o: ../rtk/include/rtk_msg.hh ../rtk/include/rtk_msg_router.hh
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
sonardevice.o: /usr/local/player0.9.1-Bleeding/include/stage.h
sonardevice.o: /usr/local/player0.9.1-Bleeding/include/messages.h
sonardevice.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
sonardevice.o: ../include/sonardevice.hh ../include/playerdevice.hh
usc_pioneer.o: ../include/world.hh ../include/image.h ../include/entity.hh
usc_pioneer.o: ../include/stage_types.hh ../include/guiexport.hh
usc_pioneer.o: /usr/local/player0.9.1-Bleeding/include/stage.h
usc_pioneer.o: /usr/local/player0.9.1-Bleeding/include/messages.h
usc_pioneer.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
usc_pioneer.o: ../include/usc_pioneer.hh ../include/playerserver.hh
usc_pioneer.o: ../include/pioneermobiledevice.hh ../include/playerdevice.hh
usc_pioneer.o: ../include/miscdevice.hh ../include/sonardevice.hh
usc_pioneer.o: ../include/laserdevice.hh ../include/laserbeacondevice.hh
usc_pioneer.o: ../include/ptzdevice.hh ../include/visiondevice.hh
visionbeacon.o: ../include/world.hh ../include/image.h ../include/entity.hh
visionbeacon.o: ../include/stage_types.hh ../include/guiexport.hh
visionbeacon.o: /usr/local/player0.9.1-Bleeding/include/stage.h
visionbeacon.o: /usr/local/player0.9.1-Bleeding/include/messages.h
visionbeacon.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
visionbeacon.o: ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh ../include/image.h ../include/entity.hh
visiondevice.o: ../include/stage_types.hh ../include/guiexport.hh
visiondevice.o: /usr/local/player0.9.1-Bleeding/include/stage.h
visiondevice.o: /usr/local/player0.9.1-Bleeding/include/messages.h
visiondevice.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
visiondevice.o: ../include/visiondevice.hh ../include/playerdevice.hh
visiondevice.o: ../include/ptzdevice.hh
world.o: /usr/local/player0.9.1-Bleeding/include/stage.h
world.o: /usr/local/player0.9.1-Bleeding/include/messages.h
world.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
world.o: ../include/world.hh ../include/image.h ../include/entity.hh
world.o: ../include/stage_types.hh ../include/guiexport.hh
world_load.o: ../include/stage_types.hh ../include/world.hh
world_load.o: ../include/image.h ../include/entity.hh ../include/guiexport.hh
world_load.o: /usr/local/player0.9.1-Bleeding/include/stage.h
world_load.o: /usr/local/player0.9.1-Bleeding/include/messages.h
world_load.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
world_load.o: ../include/entityfactory.hh
xgui.o: ../include/xgui.hh ../include/guiexport.hh
xgui.o: /usr/local/player0.9.1-Bleeding/include/stage.h
xgui.o: /usr/local/player0.9.1-Bleeding/include/messages.h
xgui.o: /usr/local/player0.9.1-Bleeding/include/playercommon.h
xgui.o: ../include/world.hh ../include/image.h ../include/entity.hh
xgui.o: ../include/stage_types.hh ../include/pioneermobiledevice.hh
xgui.o: ../include/playerdevice.hh
