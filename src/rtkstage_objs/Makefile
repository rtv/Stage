
##############################################################################
# include the main configurations from the top-level directory
#

include ../../Makefile.common

#############################################################################
# These are the key paths needed for the build

RTK_DIR = ../../rtk
# rtkstage needs RTK installed and these paths set correctly
# RTK headers
RTK_INC = $(RTK_DIR)/include
# RTK libraries
RTK_LIB = $(RTK_DIR)/lib

##############################################################################
# Build section

# Objects common to all targets
#
OBJS =  world.o world_load.o truthserver.o envserver.o image.o \
	entity.o entityfactory.o playerdevice.o \
	positiondevice.o laserdevice.o sonardevice.o miscdevice.o \
	ptzdevice.o boxobstacle.o \
	laserbeacon.o laserbeacondevice.o visiondevice.o visionbeacon.o \
	broadcastdevice.o puck.o gpsdevice.o gripperdevice.o rtk_main.o \
	matrix.o raytrace.o

# choose one of these that suits your build (debug/profile/production)

# debug
CCFLAGS = -g3 
# debug + profile
#CCFLAGS = -pg -g3 
# production
#CCFLAGS = -O3 

# Flags common to all targets
#
CCFLAGS += -Wall -DREENTRANT -I../../include -I$(PLAYER_INC) $(ZLIB_FLAGS) 
CCFLAGS += -DVERSION=\"$(VERSION)\"
CCFLAGS += -DINCLUDE_RTK -I$(RTK_INC) `gtk-config --cflags`
CCFLAGS += -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\"

# production/debug link
LINKFLAGS = -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
# profile link
#LINKFLAGS = -pg -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
LINKFLAGS += -L$(RTK_LIB) -lrtk `gtk-config --libs gtk gthread`

# Standard tools
#
CC := g++
LINKER = g++
MAKEDEP = makedepend

.SUFFIXES: .cc .o

##############################################################################
# Targets

all: rtkstage 

# Target stage with RTK GUI
#
rtkstage : $(OBJS) rtk
	$(LINKER) $(OBJS) $(LINKFLAGS) -o $@

rtk:
	cd $(RTK_DIR) && make

clean:
	rm -f *.o *~ core rtkstage

dep:
	$(MAKEDEP) -Y -- $(CCFLAGS) -- -s "# Dependancies (make dep)" ../*.cc 2>/dev/null
	$(RM) -f Makefile.bak

clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak

%.o : ../%.cc
	$(CC) $(CCFLAGS) -c $< -o $@

# Dependancies (make dep)

../boxobstacle.o: ../../include/world.hh
../boxobstacle.o: /usr/local/player-1.0-gamma/include/messages.h
../boxobstacle.o: /usr/local/player-1.0-gamma/include/playercommon.h
../boxobstacle.o: ../../include/image.hh ../../include/entity.hh
../boxobstacle.o: /usr/local/player-1.0-gamma/include/stage.h
../boxobstacle.o: ../../include/stage_types.hh ../../include/truthserver.hh
../boxobstacle.o: ../../include/colors.hh ../../include/guiexport.hh
../boxobstacle.o: ../../include/matrix.hh ../../include/boxobstacle.hh
../broadcastdevice.o: /usr/local/player-1.0-gamma/include/stage.h
../broadcastdevice.o: /usr/local/player-1.0-gamma/include/messages.h
../broadcastdevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../broadcastdevice.o: ../../include/world.hh ../../include/image.hh
../broadcastdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../broadcastdevice.o: ../../include/truthserver.hh ../../include/colors.hh
../broadcastdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../broadcastdevice.o: ../../include/broadcastdevice.hh
../broadcastdevice.o: ../../include/playerdevice.hh
../entity.o: ../../include/entity.hh
../entity.o: /usr/local/player-1.0-gamma/include/stage.h
../entity.o: /usr/local/player-1.0-gamma/include/messages.h
../entity.o: /usr/local/player-1.0-gamma/include/playercommon.h
../entity.o: ../../include/stage_types.hh ../../include/truthserver.hh
../entity.o: ../../include/colors.hh ../../include/guiexport.hh
../entity.o: ../../include/world.hh ../../include/image.hh
../entity.o: ../../include/matrix.hh
../entityfactory.o: ../../include/entityfactory.hh
../entityfactory.o: ../../include/boxobstacle.hh ../../include/entity.hh
../entityfactory.o: /usr/local/player-1.0-gamma/include/stage.h
../entityfactory.o: /usr/local/player-1.0-gamma/include/messages.h
../entityfactory.o: /usr/local/player-1.0-gamma/include/playercommon.h
../entityfactory.o: ../../include/stage_types.hh ../../include/truthserver.hh
../entityfactory.o: ../../include/colors.hh ../../include/guiexport.hh
../entityfactory.o: ../../include/laserbeacon.hh ../../include/puck.hh
../entityfactory.o: ../../include/visionbeacon.hh
../entityfactory.o: ../../include/sonardevice.hh
../entityfactory.o: ../../include/playerdevice.hh
../entityfactory.o: ../../include/positiondevice.hh
../entityfactory.o: ../../include/laserdevice.hh ../../include/miscdevice.hh
../entityfactory.o: ../../include/ptzdevice.hh ../../include/visiondevice.hh
../entityfactory.o: ../../include/laserbeacondevice.hh
../entityfactory.o: ../../include/broadcastdevice.hh
../entityfactory.o: ../../include/gripperdevice.hh ../../include/gpsdevice.hh
../entityfactory.o: ../../include/world.hh ../../include/image.hh
../entityfactory.o: ../../include/matrix.hh
../envserver.o: ../../include/world.hh
../envserver.o: /usr/local/player-1.0-gamma/include/messages.h
../envserver.o: /usr/local/player-1.0-gamma/include/playercommon.h
../envserver.o: ../../include/image.hh ../../include/entity.hh
../envserver.o: /usr/local/player-1.0-gamma/include/stage.h
../envserver.o: ../../include/stage_types.hh ../../include/truthserver.hh
../envserver.o: ../../include/colors.hh ../../include/guiexport.hh
../envserver.o: ../../include/matrix.hh
../footie.o: /usr/local/player-1.0-gamma/include/playerclient.h
../footie.o: /usr/local/player-1.0-gamma/include/messages.h
../footie.o: /usr/local/player-1.0-gamma/include/playercommon.h
../footie.o: /usr/local/player-1.0-gamma/include/playercclient.h
../footie.o: /usr/local/player-1.0-gamma/include/laserproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/clientproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/sonarproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/miscproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/positionproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/ptzproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/gripperproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/speechproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/laserbeaconproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/visionproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/truthproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/occupancyproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/gpsproxy.h
../footie.o: /usr/local/player-1.0-gamma/include/bpsproxy.h
../gpsdevice.o: /usr/local/player-1.0-gamma/include/stage.h
../gpsdevice.o: /usr/local/player-1.0-gamma/include/messages.h
../gpsdevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../gpsdevice.o: ../../include/world.hh ../../include/image.hh
../gpsdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../gpsdevice.o: ../../include/truthserver.hh ../../include/colors.hh
../gpsdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../gpsdevice.o: ../../include/gpsdevice.hh
../gripperdevice.o: ../../include/world.hh
../gripperdevice.o: /usr/local/player-1.0-gamma/include/messages.h
../gripperdevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../gripperdevice.o: ../../include/image.hh ../../include/entity.hh
../gripperdevice.o: /usr/local/player-1.0-gamma/include/stage.h
../gripperdevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../gripperdevice.o: ../../include/colors.hh ../../include/guiexport.hh
../gripperdevice.o: ../../include/matrix.hh ../../include/gripperdevice.hh
../gripperdevice.o: ../../include/raytrace.hh
../image.o: ../../include/image.hh
../laserbeacon.o: ../../include/world.hh
../laserbeacon.o: /usr/local/player-1.0-gamma/include/messages.h
../laserbeacon.o: /usr/local/player-1.0-gamma/include/playercommon.h
../laserbeacon.o: ../../include/image.hh ../../include/entity.hh
../laserbeacon.o: /usr/local/player-1.0-gamma/include/stage.h
../laserbeacon.o: ../../include/stage_types.hh ../../include/truthserver.hh
../laserbeacon.o: ../../include/colors.hh ../../include/guiexport.hh
../laserbeacon.o: ../../include/matrix.hh ../../include/laserbeacon.hh
../laserbeacon.o: ../../include/laserdevice.hh ../../include/playerdevice.hh
../laserbeacondevice.o: /usr/local/player-1.0-gamma/include/stage.h
../laserbeacondevice.o: /usr/local/player-1.0-gamma/include/messages.h
../laserbeacondevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../laserbeacondevice.o: ../../include/world.hh ../../include/image.hh
../laserbeacondevice.o: ../../include/entity.hh ../../include/stage_types.hh
../laserbeacondevice.o: ../../include/truthserver.hh ../../include/colors.hh
../laserbeacondevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../laserbeacondevice.o: ../../include/laserdevice.hh
../laserbeacondevice.o: ../../include/playerdevice.hh
../laserbeacondevice.o: ../../include/laserbeacon.hh
../laserbeacondevice.o: ../../include/laserbeacondevice.hh
../laserdevice.o: /usr/local/player-1.0-gamma/include/stage.h
../laserdevice.o: /usr/local/player-1.0-gamma/include/messages.h
../laserdevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../laserdevice.o: ../../include/world.hh ../../include/image.hh
../laserdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../laserdevice.o: ../../include/truthserver.hh ../../include/colors.hh
../laserdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../laserdevice.o: ../../include/laserdevice.hh ../../include/playerdevice.hh
../laserdevice.o: ../../include/laserbeacon.hh ../../include/raytrace.hh
../main.o: ../../include/world.hh
../main.o: /usr/local/player-1.0-gamma/include/messages.h
../main.o: /usr/local/player-1.0-gamma/include/playercommon.h
../main.o: ../../include/image.hh ../../include/entity.hh
../main.o: /usr/local/player-1.0-gamma/include/stage.h
../main.o: ../../include/stage_types.hh ../../include/truthserver.hh
../main.o: ../../include/colors.hh ../../include/guiexport.hh
../main.o: ../../include/matrix.hh
../manager.o: ../../include/truthserver.hh
../manager.o: /usr/local/player-1.0-gamma/include/messages.h
../manager.o: /usr/local/player-1.0-gamma/include/playercommon.h
../manager.o: ../../include/stage_types.hh
../matrix.o: ../../include/matrix.hh ../../include/image.hh
../matrix.o: ../../include/entity.hh
../matrix.o: /usr/local/player-1.0-gamma/include/stage.h
../matrix.o: /usr/local/player-1.0-gamma/include/messages.h
../matrix.o: /usr/local/player-1.0-gamma/include/playercommon.h
../matrix.o: ../../include/stage_types.hh ../../include/truthserver.hh
../matrix.o: ../../include/colors.hh ../../include/guiexport.hh
../miscdevice.o: /usr/local/player-1.0-gamma/include/stage.h
../miscdevice.o: /usr/local/player-1.0-gamma/include/messages.h
../miscdevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../miscdevice.o: ../../include/world.hh ../../include/image.hh
../miscdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../miscdevice.o: ../../include/truthserver.hh ../../include/colors.hh
../miscdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../miscdevice.o: ../../include/miscdevice.hh ../../include/playerdevice.hh
../occupancydevice.o: /usr/local/player-1.0-gamma/include/stage.h
../occupancydevice.o: /usr/local/player-1.0-gamma/include/messages.h
../occupancydevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../occupancydevice.o: ../../include/world.hh ../../include/image.hh
../occupancydevice.o: ../../include/entity.hh ../../include/stage_types.hh
../occupancydevice.o: ../../include/truthserver.hh ../../include/colors.hh
../occupancydevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../occupancydevice.o: ../../include/occupancydevice.hh
../playerdevice.o: /usr/local/player-1.0-gamma/include/stage.h
../playerdevice.o: /usr/local/player-1.0-gamma/include/messages.h
../playerdevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../playerdevice.o: ../../include/world.hh ../../include/image.hh
../playerdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../playerdevice.o: ../../include/truthserver.hh ../../include/colors.hh
../playerdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../playerdevice.o: ../../include/playerdevice.hh
../player_xpsi.o: /usr/X11R6/include/X11/keysym.h
../player_xpsi.o: /usr/X11R6/include/X11/keysymdef.h
../player_xpsi.o: /usr/X11R6/include/X11/Xlib.h /usr/X11R6/include/X11/X.h
../player_xpsi.o: /usr/X11R6/include/X11/Xfuncproto.h
../player_xpsi.o: /usr/X11R6/include/X11/Xosdefs.h
../player_xpsi.o: /usr/X11R6/include/X11/Xutil.h /usr/X11R6/include/X11/Xos.h
../player_xpsi.o: /usr/X11R6/include/X11/Xarch.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/playerclient.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/messages.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/playercommon.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/playercclient.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/laserproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/clientproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/sonarproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/miscproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/positionproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/ptzproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/gripperproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/speechproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/laserbeaconproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/visionproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/truthproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/occupancyproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/gpsproxy.h
../player_xpsi.o: /usr/local/player-1.0-gamma/include/bpsproxy.h
../positiondevice.o: ../../include/world.hh
../positiondevice.o: /usr/local/player-1.0-gamma/include/messages.h
../positiondevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../positiondevice.o: ../../include/image.hh ../../include/entity.hh
../positiondevice.o: /usr/local/player-1.0-gamma/include/stage.h
../positiondevice.o: ../../include/stage_types.hh
../positiondevice.o: ../../include/truthserver.hh ../../include/colors.hh
../positiondevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../positiondevice.o: ../../include/positiondevice.hh
../positiondevice.o: ../../include/raytrace.hh
../ptzdevice.o: ../../include/world.hh
../ptzdevice.o: /usr/local/player-1.0-gamma/include/messages.h
../ptzdevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../ptzdevice.o: ../../include/image.hh ../../include/entity.hh
../ptzdevice.o: /usr/local/player-1.0-gamma/include/stage.h
../ptzdevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../ptzdevice.o: ../../include/colors.hh ../../include/guiexport.hh
../ptzdevice.o: ../../include/matrix.hh ../../include/ptzdevice.hh
../ptzdevice.o: ../../include/playerdevice.hh
../puck.o: ../../include/world.hh
../puck.o: /usr/local/player-1.0-gamma/include/messages.h
../puck.o: /usr/local/player-1.0-gamma/include/playercommon.h
../puck.o: ../../include/image.hh ../../include/entity.hh
../puck.o: /usr/local/player-1.0-gamma/include/stage.h
../puck.o: ../../include/stage_types.hh ../../include/truthserver.hh
../puck.o: ../../include/colors.hh ../../include/guiexport.hh
../puck.o: ../../include/matrix.hh ../../include/puck.hh
../puck.o: ../../include/raytrace.hh
../raytrace.o: ../../include/raytrace.hh ../../include/matrix.hh
../raytrace.o: ../../include/image.hh ../../include/entity.hh
../raytrace.o: /usr/local/player-1.0-gamma/include/stage.h
../raytrace.o: /usr/local/player-1.0-gamma/include/messages.h
../raytrace.o: /usr/local/player-1.0-gamma/include/playercommon.h
../raytrace.o: ../../include/stage_types.hh ../../include/truthserver.hh
../raytrace.o: ../../include/colors.hh ../../include/guiexport.hh
../render.o: /usr/X11R6/include/X11/keysym.h
../render.o: /usr/X11R6/include/X11/keysymdef.h /usr/X11R6/include/X11/Xlib.h
../render.o: /usr/X11R6/include/X11/X.h /usr/X11R6/include/X11/Xfuncproto.h
../render.o: /usr/X11R6/include/X11/Xosdefs.h /usr/X11R6/include/X11/Xutil.h
../render.o: /usr/X11R6/include/X11/Xos.h /usr/X11R6/include/X11/Xarch.h
../render.o: ../../include/stage_types.hh ../../include/truthserver.hh
../render.o: /usr/local/player-1.0-gamma/include/messages.h
../render.o: /usr/local/player-1.0-gamma/include/playercommon.h
../render.o: ../../include/xs.hh
../render.o: /usr/local/player-1.0-gamma/include/playermulticlient.h
../render.o: /usr/local/player-1.0-gamma/include/playerclient.h
../render.o: /usr/local/player-1.0-gamma/include/playercclient.h
../render.o: /usr/local/player-1.0-gamma/include/laserproxy.h
../render.o: /usr/local/player-1.0-gamma/include/clientproxy.h
../render.o: /usr/local/player-1.0-gamma/include/sonarproxy.h
../render.o: /usr/local/player-1.0-gamma/include/miscproxy.h
../render.o: /usr/local/player-1.0-gamma/include/positionproxy.h
../render.o: /usr/local/player-1.0-gamma/include/ptzproxy.h
../render.o: /usr/local/player-1.0-gamma/include/gripperproxy.h
../render.o: /usr/local/player-1.0-gamma/include/speechproxy.h
../render.o: /usr/local/player-1.0-gamma/include/laserbeaconproxy.h
../render.o: /usr/local/player-1.0-gamma/include/visionproxy.h
../render.o: /usr/local/player-1.0-gamma/include/truthproxy.h
../render.o: /usr/local/player-1.0-gamma/include/occupancyproxy.h
../render.o: /usr/local/player-1.0-gamma/include/gpsproxy.h
../render.o: /usr/local/player-1.0-gamma/include/bpsproxy.h
../rtk_main.o: ../../include/world.hh
../rtk_main.o: /usr/local/player-1.0-gamma/include/messages.h
../rtk_main.o: /usr/local/player-1.0-gamma/include/playercommon.h
../rtk_main.o: ../../include/image.hh ../../include/entity.hh
../rtk_main.o: /usr/local/player-1.0-gamma/include/stage.h
../rtk_main.o: ../../include/stage_types.hh ../../include/truthserver.hh
../rtk_main.o: ../../include/colors.hh ../../include/guiexport.hh
../rtk_main.o: ../../include/matrix.hh
../sonardevice.o: ../../include/world.hh
../sonardevice.o: /usr/local/player-1.0-gamma/include/messages.h
../sonardevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../sonardevice.o: ../../include/image.hh ../../include/entity.hh
../sonardevice.o: /usr/local/player-1.0-gamma/include/stage.h
../sonardevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../sonardevice.o: ../../include/colors.hh ../../include/guiexport.hh
../sonardevice.o: ../../include/matrix.hh ../../include/sonardevice.hh
../sonardevice.o: ../../include/playerdevice.hh ../../include/raytrace.hh
../truthdevice.o: ../../include/world.hh
../truthdevice.o: /usr/local/player-1.0-gamma/include/messages.h
../truthdevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../truthdevice.o: ../../include/image.hh ../../include/entity.hh
../truthdevice.o: /usr/local/player-1.0-gamma/include/stage.h
../truthdevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../truthdevice.o: ../../include/colors.hh ../../include/guiexport.hh
../truthdevice.o: ../../include/matrix.hh ../../include/truthdevice.hh
../truthlog.o: ../../include/truthserver.hh
../truthlog.o: /usr/local/player-1.0-gamma/include/messages.h
../truthlog.o: /usr/local/player-1.0-gamma/include/playercommon.h
../truthlog.o: ../../include/stage_types.hh
../truthserver.o: ../../include/world.hh
../truthserver.o: /usr/local/player-1.0-gamma/include/messages.h
../truthserver.o: /usr/local/player-1.0-gamma/include/playercommon.h
../truthserver.o: ../../include/image.hh ../../include/entity.hh
../truthserver.o: /usr/local/player-1.0-gamma/include/stage.h
../truthserver.o: ../../include/stage_types.hh ../../include/truthserver.hh
../truthserver.o: ../../include/colors.hh ../../include/guiexport.hh
../truthserver.o: ../../include/matrix.hh
../visionbeacon.o: ../../include/world.hh
../visionbeacon.o: /usr/local/player-1.0-gamma/include/messages.h
../visionbeacon.o: /usr/local/player-1.0-gamma/include/playercommon.h
../visionbeacon.o: ../../include/image.hh ../../include/entity.hh
../visionbeacon.o: /usr/local/player-1.0-gamma/include/stage.h
../visionbeacon.o: ../../include/stage_types.hh ../../include/truthserver.hh
../visionbeacon.o: ../../include/colors.hh ../../include/guiexport.hh
../visionbeacon.o: ../../include/matrix.hh ../../include/visionbeacon.hh
../visiondevice.o: ../../include/world.hh
../visiondevice.o: /usr/local/player-1.0-gamma/include/messages.h
../visiondevice.o: /usr/local/player-1.0-gamma/include/playercommon.h
../visiondevice.o: ../../include/image.hh ../../include/entity.hh
../visiondevice.o: /usr/local/player-1.0-gamma/include/stage.h
../visiondevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../visiondevice.o: ../../include/colors.hh ../../include/guiexport.hh
../visiondevice.o: ../../include/matrix.hh ../../include/visiondevice.hh
../visiondevice.o: ../../include/ptzdevice.hh ../../include/playerdevice.hh
../visiondevice.o: ../../include/raytrace.hh
../world.o: ../../include/world.hh
../world.o: /usr/local/player-1.0-gamma/include/messages.h
../world.o: /usr/local/player-1.0-gamma/include/playercommon.h
../world.o: ../../include/image.hh ../../include/entity.hh
../world.o: /usr/local/player-1.0-gamma/include/stage.h
../world.o: ../../include/stage_types.hh ../../include/truthserver.hh
../world.o: ../../include/colors.hh ../../include/guiexport.hh
../world.o: ../../include/matrix.hh ../../include/playerdevice.hh
../world_load.o: ../../include/stage_types.hh ../../include/world.hh
../world_load.o: /usr/local/player-1.0-gamma/include/messages.h
../world_load.o: /usr/local/player-1.0-gamma/include/playercommon.h
../world_load.o: ../../include/image.hh ../../include/entity.hh
../world_load.o: /usr/local/player-1.0-gamma/include/stage.h
../world_load.o: ../../include/truthserver.hh ../../include/colors.hh
../world_load.o: ../../include/guiexport.hh ../../include/matrix.hh
../world_load.o: ../../include/entityfactory.hh
../xs.o: /usr/X11R6/include/X11/keysym.h /usr/X11R6/include/X11/keysymdef.h
../xs.o: /usr/X11R6/include/X11/Xlib.h /usr/X11R6/include/X11/X.h
../xs.o: /usr/X11R6/include/X11/Xfuncproto.h /usr/X11R6/include/X11/Xosdefs.h
../xs.o: /usr/X11R6/include/X11/Xutil.h /usr/X11R6/include/X11/Xos.h
../xs.o: /usr/X11R6/include/X11/Xarch.h ../../include/stage_types.hh
../xs.o: ../../include/truthserver.hh
../xs.o: /usr/local/player-1.0-gamma/include/messages.h
../xs.o: /usr/local/player-1.0-gamma/include/playercommon.h
../xs.o: ../../include/xs.hh
../xs.o: /usr/local/player-1.0-gamma/include/playermulticlient.h
../xs.o: /usr/local/player-1.0-gamma/include/playerclient.h
../xs.o: /usr/local/player-1.0-gamma/include/playercclient.h
../xs.o: /usr/local/player-1.0-gamma/include/laserproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/clientproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/sonarproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/miscproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/positionproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/ptzproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/gripperproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/speechproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/laserbeaconproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/visionproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/truthproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/occupancyproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/gpsproxy.h
../xs.o: /usr/local/player-1.0-gamma/include/bpsproxy.h
