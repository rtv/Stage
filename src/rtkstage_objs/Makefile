
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
../boxobstacle.o: /home/vaughan/PS/player/include/messages.h
../boxobstacle.o: /home/vaughan/PS/player/include/playercommon.h
../boxobstacle.o: ../../include/image.hh ../../include/entity.hh
../boxobstacle.o: /home/vaughan/PS/player/include/stage.h
../boxobstacle.o: ../../include/stage_types.hh ../../include/truthserver.hh
../boxobstacle.o: ../../include/colors.hh ../../include/guiexport.hh
../boxobstacle.o: ../../include/matrix.hh ../../include/boxobstacle.hh
../broadcastdevice.o: /home/vaughan/PS/player/include/stage.h
../broadcastdevice.o: /home/vaughan/PS/player/include/messages.h
../broadcastdevice.o: /home/vaughan/PS/player/include/playercommon.h
../broadcastdevice.o: ../../include/world.hh ../../include/image.hh
../broadcastdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../broadcastdevice.o: ../../include/truthserver.hh ../../include/colors.hh
../broadcastdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../broadcastdevice.o: ../../include/broadcastdevice.hh
../broadcastdevice.o: ../../include/playerdevice.hh
../entity.o: ../../include/entity.hh /home/vaughan/PS/player/include/stage.h
../entity.o: /home/vaughan/PS/player/include/messages.h
../entity.o: /home/vaughan/PS/player/include/playercommon.h
../entity.o: ../../include/stage_types.hh ../../include/truthserver.hh
../entity.o: ../../include/colors.hh ../../include/guiexport.hh
../entity.o: ../../include/world.hh ../../include/image.hh
../entity.o: ../../include/matrix.hh
../entityfactory.o: ../../include/entityfactory.hh
../entityfactory.o: ../../include/boxobstacle.hh ../../include/entity.hh
../entityfactory.o: /home/vaughan/PS/player/include/stage.h
../entityfactory.o: /home/vaughan/PS/player/include/messages.h
../entityfactory.o: /home/vaughan/PS/player/include/playercommon.h
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
../envserver.o: /home/vaughan/PS/player/include/messages.h
../envserver.o: /home/vaughan/PS/player/include/playercommon.h
../envserver.o: ../../include/image.hh ../../include/entity.hh
../envserver.o: /home/vaughan/PS/player/include/stage.h
../envserver.o: ../../include/stage_types.hh ../../include/truthserver.hh
../envserver.o: ../../include/colors.hh ../../include/guiexport.hh
../envserver.o: ../../include/matrix.hh
../gpsdevice.o: /home/vaughan/PS/player/include/stage.h
../gpsdevice.o: /home/vaughan/PS/player/include/messages.h
../gpsdevice.o: /home/vaughan/PS/player/include/playercommon.h
../gpsdevice.o: ../../include/world.hh ../../include/image.hh
../gpsdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../gpsdevice.o: ../../include/truthserver.hh ../../include/colors.hh
../gpsdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../gpsdevice.o: ../../include/gpsdevice.hh
../gripperdevice.o: ../../include/world.hh
../gripperdevice.o: /home/vaughan/PS/player/include/messages.h
../gripperdevice.o: /home/vaughan/PS/player/include/playercommon.h
../gripperdevice.o: ../../include/image.hh ../../include/entity.hh
../gripperdevice.o: /home/vaughan/PS/player/include/stage.h
../gripperdevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../gripperdevice.o: ../../include/colors.hh ../../include/guiexport.hh
../gripperdevice.o: ../../include/matrix.hh ../../include/gripperdevice.hh
../gripperdevice.o: ../../include/raytrace.hh
../image.o: ../../include/image.hh
../laserbeacon.o: ../../include/world.hh
../laserbeacon.o: /home/vaughan/PS/player/include/messages.h
../laserbeacon.o: /home/vaughan/PS/player/include/playercommon.h
../laserbeacon.o: ../../include/image.hh ../../include/entity.hh
../laserbeacon.o: /home/vaughan/PS/player/include/stage.h
../laserbeacon.o: ../../include/stage_types.hh ../../include/truthserver.hh
../laserbeacon.o: ../../include/colors.hh ../../include/guiexport.hh
../laserbeacon.o: ../../include/matrix.hh ../../include/laserbeacon.hh
../laserbeacon.o: ../../include/laserdevice.hh ../../include/playerdevice.hh
../laserbeacondevice.o: /home/vaughan/PS/player/include/stage.h
../laserbeacondevice.o: /home/vaughan/PS/player/include/messages.h
../laserbeacondevice.o: /home/vaughan/PS/player/include/playercommon.h
../laserbeacondevice.o: ../../include/world.hh ../../include/image.hh
../laserbeacondevice.o: ../../include/entity.hh ../../include/stage_types.hh
../laserbeacondevice.o: ../../include/truthserver.hh ../../include/colors.hh
../laserbeacondevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../laserbeacondevice.o: ../../include/laserdevice.hh
../laserbeacondevice.o: ../../include/playerdevice.hh
../laserbeacondevice.o: ../../include/laserbeacon.hh
../laserbeacondevice.o: ../../include/laserbeacondevice.hh
../laserdevice.o: /home/vaughan/PS/player/include/stage.h
../laserdevice.o: /home/vaughan/PS/player/include/messages.h
../laserdevice.o: /home/vaughan/PS/player/include/playercommon.h
../laserdevice.o: ../../include/world.hh ../../include/image.hh
../laserdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../laserdevice.o: ../../include/truthserver.hh ../../include/colors.hh
../laserdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../laserdevice.o: ../../include/laserdevice.hh ../../include/playerdevice.hh
../laserdevice.o: ../../include/laserbeacon.hh ../../include/raytrace.hh
../main.o: ../../include/world.hh /home/vaughan/PS/player/include/messages.h
../main.o: /home/vaughan/PS/player/include/playercommon.h
../main.o: ../../include/image.hh ../../include/entity.hh
../main.o: /home/vaughan/PS/player/include/stage.h
../main.o: ../../include/stage_types.hh ../../include/truthserver.hh
../main.o: ../../include/colors.hh ../../include/guiexport.hh
../main.o: ../../include/matrix.hh
../manager.o: ../../include/truthserver.hh
../manager.o: /home/vaughan/PS/player/include/messages.h
../manager.o: /home/vaughan/PS/player/include/playercommon.h
../manager.o: ../../include/stage_types.hh
../matrix.o: ../../include/matrix.hh ../../include/image.hh
../matrix.o: ../../include/entity.hh /home/vaughan/PS/player/include/stage.h
../matrix.o: /home/vaughan/PS/player/include/messages.h
../matrix.o: /home/vaughan/PS/player/include/playercommon.h
../matrix.o: ../../include/stage_types.hh ../../include/truthserver.hh
../matrix.o: ../../include/colors.hh ../../include/guiexport.hh
../miscdevice.o: /home/vaughan/PS/player/include/stage.h
../miscdevice.o: /home/vaughan/PS/player/include/messages.h
../miscdevice.o: /home/vaughan/PS/player/include/playercommon.h
../miscdevice.o: ../../include/world.hh ../../include/image.hh
../miscdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../miscdevice.o: ../../include/truthserver.hh ../../include/colors.hh
../miscdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../miscdevice.o: ../../include/miscdevice.hh ../../include/playerdevice.hh
../occupancydevice.o: /home/vaughan/PS/player/include/stage.h
../occupancydevice.o: /home/vaughan/PS/player/include/messages.h
../occupancydevice.o: /home/vaughan/PS/player/include/playercommon.h
../occupancydevice.o: ../../include/world.hh ../../include/image.hh
../occupancydevice.o: ../../include/entity.hh ../../include/stage_types.hh
../occupancydevice.o: ../../include/truthserver.hh ../../include/colors.hh
../occupancydevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../occupancydevice.o: ../../include/occupancydevice.hh
../playerdevice.o: /home/vaughan/PS/player/include/stage.h
../playerdevice.o: /home/vaughan/PS/player/include/messages.h
../playerdevice.o: /home/vaughan/PS/player/include/playercommon.h
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
../positiondevice.o: ../../include/world.hh
../positiondevice.o: /home/vaughan/PS/player/include/messages.h
../positiondevice.o: /home/vaughan/PS/player/include/playercommon.h
../positiondevice.o: ../../include/image.hh ../../include/entity.hh
../positiondevice.o: /home/vaughan/PS/player/include/stage.h
../positiondevice.o: ../../include/stage_types.hh
../positiondevice.o: ../../include/truthserver.hh ../../include/colors.hh
../positiondevice.o: ../../include/guiexport.hh ../../include/matrix.hh
../positiondevice.o: ../../include/positiondevice.hh
../positiondevice.o: ../../include/raytrace.hh
../ptzdevice.o: ../../include/world.hh
../ptzdevice.o: /home/vaughan/PS/player/include/messages.h
../ptzdevice.o: /home/vaughan/PS/player/include/playercommon.h
../ptzdevice.o: ../../include/image.hh ../../include/entity.hh
../ptzdevice.o: /home/vaughan/PS/player/include/stage.h
../ptzdevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../ptzdevice.o: ../../include/colors.hh ../../include/guiexport.hh
../ptzdevice.o: ../../include/matrix.hh ../../include/ptzdevice.hh
../ptzdevice.o: ../../include/playerdevice.hh
../puck.o: ../../include/world.hh /home/vaughan/PS/player/include/messages.h
../puck.o: /home/vaughan/PS/player/include/playercommon.h
../puck.o: ../../include/image.hh ../../include/entity.hh
../puck.o: /home/vaughan/PS/player/include/stage.h
../puck.o: ../../include/stage_types.hh ../../include/truthserver.hh
../puck.o: ../../include/colors.hh ../../include/guiexport.hh
../puck.o: ../../include/matrix.hh ../../include/puck.hh
../puck.o: ../../include/raytrace.hh
../raytrace.o: ../../include/raytrace.hh ../../include/matrix.hh
../raytrace.o: ../../include/image.hh ../../include/entity.hh
../raytrace.o: /home/vaughan/PS/player/include/stage.h
../raytrace.o: /home/vaughan/PS/player/include/messages.h
../raytrace.o: /home/vaughan/PS/player/include/playercommon.h
../raytrace.o: ../../include/stage_types.hh ../../include/truthserver.hh
../raytrace.o: ../../include/colors.hh ../../include/guiexport.hh
../render.o: /usr/X11R6/include/X11/keysym.h
../render.o: /usr/X11R6/include/X11/keysymdef.h /usr/X11R6/include/X11/Xlib.h
../render.o: /usr/X11R6/include/X11/X.h /usr/X11R6/include/X11/Xfuncproto.h
../render.o: /usr/X11R6/include/X11/Xosdefs.h /usr/X11R6/include/X11/Xutil.h
../render.o: /usr/X11R6/include/X11/Xos.h /usr/X11R6/include/X11/Xarch.h
../render.o: ../../include/stage_types.hh ../../include/truthserver.hh
../render.o: /home/vaughan/PS/player/include/messages.h
../render.o: /home/vaughan/PS/player/include/playercommon.h
../render.o: ../../include/xs.hh
../rtk_main.o: ../../include/world.hh
../rtk_main.o: /home/vaughan/PS/player/include/messages.h
../rtk_main.o: /home/vaughan/PS/player/include/playercommon.h
../rtk_main.o: ../../include/image.hh ../../include/entity.hh
../rtk_main.o: /home/vaughan/PS/player/include/stage.h
../rtk_main.o: ../../include/stage_types.hh ../../include/truthserver.hh
../rtk_main.o: ../../include/colors.hh ../../include/guiexport.hh
../rtk_main.o: ../../include/matrix.hh
../sonardevice.o: ../../include/world.hh
../sonardevice.o: /home/vaughan/PS/player/include/messages.h
../sonardevice.o: /home/vaughan/PS/player/include/playercommon.h
../sonardevice.o: ../../include/image.hh ../../include/entity.hh
../sonardevice.o: /home/vaughan/PS/player/include/stage.h
../sonardevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../sonardevice.o: ../../include/colors.hh ../../include/guiexport.hh
../sonardevice.o: ../../include/matrix.hh ../../include/sonardevice.hh
../sonardevice.o: ../../include/playerdevice.hh ../../include/raytrace.hh
../truthdevice.o: ../../include/world.hh
../truthdevice.o: /home/vaughan/PS/player/include/messages.h
../truthdevice.o: /home/vaughan/PS/player/include/playercommon.h
../truthdevice.o: ../../include/image.hh ../../include/entity.hh
../truthdevice.o: /home/vaughan/PS/player/include/stage.h
../truthdevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../truthdevice.o: ../../include/colors.hh ../../include/guiexport.hh
../truthdevice.o: ../../include/matrix.hh ../../include/truthdevice.hh
../truthlog.o: ../../include/truthserver.hh
../truthlog.o: /home/vaughan/PS/player/include/messages.h
../truthlog.o: /home/vaughan/PS/player/include/playercommon.h
../truthlog.o: ../../include/stage_types.hh
../truthserver.o: ../../include/world.hh
../truthserver.o: /home/vaughan/PS/player/include/messages.h
../truthserver.o: /home/vaughan/PS/player/include/playercommon.h
../truthserver.o: ../../include/image.hh ../../include/entity.hh
../truthserver.o: /home/vaughan/PS/player/include/stage.h
../truthserver.o: ../../include/stage_types.hh ../../include/truthserver.hh
../truthserver.o: ../../include/colors.hh ../../include/guiexport.hh
../truthserver.o: ../../include/matrix.hh
../visionbeacon.o: ../../include/world.hh
../visionbeacon.o: /home/vaughan/PS/player/include/messages.h
../visionbeacon.o: /home/vaughan/PS/player/include/playercommon.h
../visionbeacon.o: ../../include/image.hh ../../include/entity.hh
../visionbeacon.o: /home/vaughan/PS/player/include/stage.h
../visionbeacon.o: ../../include/stage_types.hh ../../include/truthserver.hh
../visionbeacon.o: ../../include/colors.hh ../../include/guiexport.hh
../visionbeacon.o: ../../include/matrix.hh ../../include/visionbeacon.hh
../visiondevice.o: ../../include/world.hh
../visiondevice.o: /home/vaughan/PS/player/include/messages.h
../visiondevice.o: /home/vaughan/PS/player/include/playercommon.h
../visiondevice.o: ../../include/image.hh ../../include/entity.hh
../visiondevice.o: /home/vaughan/PS/player/include/stage.h
../visiondevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../visiondevice.o: ../../include/colors.hh ../../include/guiexport.hh
../visiondevice.o: ../../include/matrix.hh ../../include/visiondevice.hh
../visiondevice.o: ../../include/ptzdevice.hh ../../include/playerdevice.hh
../visiondevice.o: ../../include/raytrace.hh
../world.o: ../../include/world.hh /home/vaughan/PS/player/include/messages.h
../world.o: /home/vaughan/PS/player/include/playercommon.h
../world.o: ../../include/image.hh ../../include/entity.hh
../world.o: /home/vaughan/PS/player/include/stage.h
../world.o: ../../include/stage_types.hh ../../include/truthserver.hh
../world.o: ../../include/colors.hh ../../include/guiexport.hh
../world.o: ../../include/matrix.hh ../../include/playerdevice.hh
../world_load.o: ../../include/stage_types.hh ../../include/world.hh
../world_load.o: /home/vaughan/PS/player/include/messages.h
../world_load.o: /home/vaughan/PS/player/include/playercommon.h
../world_load.o: ../../include/image.hh ../../include/entity.hh
../world_load.o: /home/vaughan/PS/player/include/stage.h
../world_load.o: ../../include/truthserver.hh ../../include/colors.hh
../world_load.o: ../../include/guiexport.hh ../../include/matrix.hh
../world_load.o: ../../include/entityfactory.hh
../xs.o: /usr/X11R6/include/X11/keysym.h /usr/X11R6/include/X11/keysymdef.h
../xs.o: /usr/X11R6/include/X11/Xlib.h /usr/X11R6/include/X11/X.h
../xs.o: /usr/X11R6/include/X11/Xfuncproto.h /usr/X11R6/include/X11/Xosdefs.h
../xs.o: /usr/X11R6/include/X11/Xutil.h /usr/X11R6/include/X11/Xos.h
../xs.o: /usr/X11R6/include/X11/Xarch.h ../../include/stage_types.hh
../xs.o: ../../include/truthserver.hh
../xs.o: /home/vaughan/PS/player/include/messages.h
../xs.o: /home/vaughan/PS/player/include/playercommon.h ../../include/xs.hh
