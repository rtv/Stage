
##############################################################################
# include the main configurations from the top-level directory
#

include ../../Makefile.common

##############################################################################
# Build section

# Objects common to all targets
#
OBJS =  world.o world_load.o truthserver.o envserver.o image.o \
	entity.o entityfactory.o playerdevice.o \
	positiondevice.o laserdevice.o sonardevice.o miscdevice.o \
	ptzdevice.o truthdevice.o occupancydevice.o boxobstacle.o \
	laserbeacon.o laserbeacondevice.o visiondevice.o visionbeacon.o \
	broadcastdevice.o puck.o gpsdevice.o gripperdevice.o main.o

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

# production/debug link
LINKFLAGS = -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
# profile link
#LINKFLAGS = -pg -lm -lstdc++ -lpthread  $(ZLIB_LIBS)

#.SUFFIXES: .cc .o

##############################################################################
# Targets

all: stage

# Target stage with no GUI
#
stage : $(OBJS)
	$(LINKER) $(LINKFLAGS) -o $@ $(OBJS)  

clean:
	rm -f *.o *~ core stage

dep:
	$(MAKEDEP) -Y -- $(CCFLAGS) -- -s "# Dependancies (make dep)" ../*.cc 2>/dev/null
	$(RM) -f Makefile.bak

%.o : ../%.cc
	${CC} ${CCFLAGS} -c $< -o $@

##############################################################################
# Dependancies (make dep)

../boxobstacle.o: ../../include/world.hh ../../include/image.h
../boxobstacle.o: ../../include/entity.hh
../boxobstacle.o: /usr/local/player-1.0a/include/stage.h
../boxobstacle.o: /usr/local/player-1.0a/include/messages.h
../boxobstacle.o: /usr/local/player-1.0a/include/playercommon.h
../boxobstacle.o: ../../include/stage_types.hh ../../include/truthserver.hh
../boxobstacle.o: ../../include/guiexport.hh ../../include/boxobstacle.hh
../broadcastdevice.o: /usr/local/player-1.0a/include/stage.h
../broadcastdevice.o: /usr/local/player-1.0a/include/messages.h
../broadcastdevice.o: /usr/local/player-1.0a/include/playercommon.h
../broadcastdevice.o: ../../include/world.hh ../../include/image.h
../broadcastdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../broadcastdevice.o: ../../include/truthserver.hh ../../include/guiexport.hh
../broadcastdevice.o: ../../include/broadcastdevice.hh
../broadcastdevice.o: ../../include/playerdevice.hh
../entity.o: ../../include/entity.hh /usr/local/player-1.0a/include/stage.h
../entity.o: /usr/local/player-1.0a/include/messages.h
../entity.o: /usr/local/player-1.0a/include/playercommon.h
../entity.o: ../../include/stage_types.hh ../../include/truthserver.hh
../entity.o: ../../include/guiexport.hh ../../include/world.hh
../entity.o: ../../include/image.h
../entityfactory.o: ../../include/entityfactory.hh
../entityfactory.o: ../../include/boxobstacle.hh ../../include/entity.hh
../entityfactory.o: /usr/local/player-1.0a/include/stage.h
../entityfactory.o: /usr/local/player-1.0a/include/messages.h
../entityfactory.o: /usr/local/player-1.0a/include/playercommon.h
../entityfactory.o: ../../include/stage_types.hh ../../include/truthserver.hh
../entityfactory.o: ../../include/guiexport.hh ../../include/laserbeacon.hh
../entityfactory.o: ../../include/puck.hh ../../include/visionbeacon.hh
../entityfactory.o: ../../include/sonardevice.hh
../entityfactory.o: ../../include/playerdevice.hh
../entityfactory.o: ../../include/positiondevice.hh
../entityfactory.o: ../../include/laserdevice.hh ../../include/miscdevice.hh
../entityfactory.o: ../../include/ptzdevice.hh ../../include/visiondevice.hh
../entityfactory.o: ../../include/laserbeacondevice.hh
../entityfactory.o: ../../include/broadcastdevice.hh
../entityfactory.o: ../../include/gripperdevice.hh ../../include/gpsdevice.hh
../envserver.o: ../../include/world.hh ../../include/image.h
../envserver.o: ../../include/entity.hh
../envserver.o: /usr/local/player-1.0a/include/stage.h
../envserver.o: /usr/local/player-1.0a/include/messages.h
../envserver.o: /usr/local/player-1.0a/include/playercommon.h
../envserver.o: ../../include/stage_types.hh ../../include/truthserver.hh
../envserver.o: ../../include/guiexport.hh
../gpsdevice.o: /usr/local/player-1.0a/include/stage.h
../gpsdevice.o: /usr/local/player-1.0a/include/messages.h
../gpsdevice.o: /usr/local/player-1.0a/include/playercommon.h
../gpsdevice.o: ../../include/world.hh ../../include/image.h
../gpsdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../gpsdevice.o: ../../include/truthserver.hh ../../include/guiexport.hh
../gpsdevice.o: ../../include/gpsdevice.hh
../gripperdevice.o: ../../include/world.hh ../../include/image.h
../gripperdevice.o: ../../include/entity.hh
../gripperdevice.o: /usr/local/player-1.0a/include/stage.h
../gripperdevice.o: /usr/local/player-1.0a/include/messages.h
../gripperdevice.o: /usr/local/player-1.0a/include/playercommon.h
../gripperdevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../gripperdevice.o: ../../include/guiexport.hh ../../include/gripperdevice.hh
../image.o: ../../include/image.h
../laserbeacon.o: ../../include/world.hh ../../include/image.h
../laserbeacon.o: ../../include/entity.hh
../laserbeacon.o: /usr/local/player-1.0a/include/stage.h
../laserbeacon.o: /usr/local/player-1.0a/include/messages.h
../laserbeacon.o: /usr/local/player-1.0a/include/playercommon.h
../laserbeacon.o: ../../include/stage_types.hh ../../include/truthserver.hh
../laserbeacon.o: ../../include/guiexport.hh ../../include/laserbeacon.hh
../laserbeacondevice.o: /usr/local/player-1.0a/include/stage.h
../laserbeacondevice.o: /usr/local/player-1.0a/include/messages.h
../laserbeacondevice.o: /usr/local/player-1.0a/include/playercommon.h
../laserbeacondevice.o: ../../include/world.hh ../../include/image.h
../laserbeacondevice.o: ../../include/entity.hh ../../include/stage_types.hh
../laserbeacondevice.o: ../../include/truthserver.hh
../laserbeacondevice.o: ../../include/guiexport.hh
../laserbeacondevice.o: ../../include/laserdevice.hh
../laserbeacondevice.o: ../../include/playerdevice.hh
../laserbeacondevice.o: ../../include/laserbeacondevice.hh
../laserdevice.o: /usr/local/player-1.0a/include/stage.h
../laserdevice.o: /usr/local/player-1.0a/include/messages.h
../laserdevice.o: /usr/local/player-1.0a/include/playercommon.h
../laserdevice.o: ../../include/world.hh ../../include/image.h
../laserdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../laserdevice.o: ../../include/truthserver.hh ../../include/guiexport.hh
../laserdevice.o: ../../include/laserdevice.hh ../../include/playerdevice.hh
../main.o: ../../include/world.hh ../../include/image.h
../main.o: ../../include/entity.hh /usr/local/player-1.0a/include/stage.h
../main.o: /usr/local/player-1.0a/include/messages.h
../main.o: /usr/local/player-1.0a/include/playercommon.h
../main.o: ../../include/stage_types.hh ../../include/truthserver.hh
../main.o: ../../include/guiexport.hh
../miscdevice.o: /usr/local/player-1.0a/include/stage.h
../miscdevice.o: /usr/local/player-1.0a/include/messages.h
../miscdevice.o: /usr/local/player-1.0a/include/playercommon.h
../miscdevice.o: ../../include/world.hh ../../include/image.h
../miscdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../miscdevice.o: ../../include/truthserver.hh ../../include/guiexport.hh
../miscdevice.o: ../../include/miscdevice.hh ../../include/playerdevice.hh
../occupancydevice.o: /usr/local/player-1.0a/include/stage.h
../occupancydevice.o: /usr/local/player-1.0a/include/messages.h
../occupancydevice.o: /usr/local/player-1.0a/include/playercommon.h
../occupancydevice.o: ../../include/world.hh ../../include/image.h
../occupancydevice.o: ../../include/entity.hh ../../include/stage_types.hh
../occupancydevice.o: ../../include/truthserver.hh ../../include/guiexport.hh
../occupancydevice.o: ../../include/occupancydevice.hh
../playerdevice.o: /usr/local/player-1.0a/include/stage.h
../playerdevice.o: /usr/local/player-1.0a/include/messages.h
../playerdevice.o: /usr/local/player-1.0a/include/playercommon.h
../playerdevice.o: ../../include/world.hh ../../include/image.h
../playerdevice.o: ../../include/entity.hh ../../include/stage_types.hh
../playerdevice.o: ../../include/truthserver.hh ../../include/guiexport.hh
../playerdevice.o: ../../include/playerdevice.hh
../player_xpsi.o: /usr/local/player-1.0a/include/playerclient.h
../player_xpsi.o: /usr/local/player-1.0a/include/messages.h
../player_xpsi.o: /usr/local/player-1.0a/include/playercommon.h
../player_xpsi.o: /usr/local/player-1.0a/include/playercclient.h
../player_xpsi.o: /usr/local/player-1.0a/include/laserproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/clientproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/sonarproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/miscproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/positionproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/ptzproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/gripperproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/speechproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/laserbeaconproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/visionproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/truthproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/occupancyproxy.h
../player_xpsi.o: /usr/local/player-1.0a/include/gpsproxy.h
../positiondevice.o: ../../include/world.hh ../../include/image.h
../positiondevice.o: ../../include/entity.hh
../positiondevice.o: /usr/local/player-1.0a/include/stage.h
../positiondevice.o: /usr/local/player-1.0a/include/messages.h
../positiondevice.o: /usr/local/player-1.0a/include/playercommon.h
../positiondevice.o: ../../include/stage_types.hh
../positiondevice.o: ../../include/truthserver.hh ../../include/guiexport.hh
../positiondevice.o: ../../include/positiondevice.hh
../ptzdevice.o: ../../include/world.hh ../../include/image.h
../ptzdevice.o: ../../include/entity.hh
../ptzdevice.o: /usr/local/player-1.0a/include/stage.h
../ptzdevice.o: /usr/local/player-1.0a/include/messages.h
../ptzdevice.o: /usr/local/player-1.0a/include/playercommon.h
../ptzdevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../ptzdevice.o: ../../include/guiexport.hh ../../include/ptzdevice.hh
../ptzdevice.o: ../../include/playerdevice.hh
../puck.o: ../../include/world.hh ../../include/image.h
../puck.o: ../../include/entity.hh /usr/local/player-1.0a/include/stage.h
../puck.o: /usr/local/player-1.0a/include/messages.h
../puck.o: /usr/local/player-1.0a/include/playercommon.h
../puck.o: ../../include/stage_types.hh ../../include/truthserver.hh
../puck.o: ../../include/guiexport.hh ../../include/puck.hh
../render.o: ../../include/stage_types.hh ../../include/truthserver.hh
../render.o: /usr/local/player-1.0a/include/messages.h
../render.o: /usr/local/player-1.0a/include/playercommon.h
../render.o: ../../include/xs.hh
../rtk_main.o: ../../include/world.hh ../../include/image.h
../rtk_main.o: ../../include/entity.hh /usr/local/player-1.0a/include/stage.h
../rtk_main.o: /usr/local/player-1.0a/include/messages.h
../rtk_main.o: /usr/local/player-1.0a/include/playercommon.h
../rtk_main.o: ../../include/stage_types.hh ../../include/truthserver.hh
../rtk_main.o: ../../include/guiexport.hh
../sonardevice.o: ../../include/world.hh ../../include/image.h
../sonardevice.o: ../../include/entity.hh
../sonardevice.o: /usr/local/player-1.0a/include/stage.h
../sonardevice.o: /usr/local/player-1.0a/include/messages.h
../sonardevice.o: /usr/local/player-1.0a/include/playercommon.h
../sonardevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../sonardevice.o: ../../include/guiexport.hh ../../include/sonardevice.hh
../sonardevice.o: ../../include/playerdevice.hh
../truthdevice.o: ../../include/world.hh ../../include/image.h
../truthdevice.o: ../../include/entity.hh
../truthdevice.o: /usr/local/player-1.0a/include/stage.h
../truthdevice.o: /usr/local/player-1.0a/include/messages.h
../truthdevice.o: /usr/local/player-1.0a/include/playercommon.h
../truthdevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../truthdevice.o: ../../include/guiexport.hh ../../include/truthdevice.hh
../truthlog.o: ../../include/truthserver.hh
../truthlog.o: /usr/local/player-1.0a/include/messages.h
../truthlog.o: /usr/local/player-1.0a/include/playercommon.h
../truthlog.o: ../../include/stage_types.hh
../truthserver.o: ../../include/world.hh ../../include/image.h
../truthserver.o: ../../include/entity.hh
../truthserver.o: /usr/local/player-1.0a/include/stage.h
../truthserver.o: /usr/local/player-1.0a/include/messages.h
../truthserver.o: /usr/local/player-1.0a/include/playercommon.h
../truthserver.o: ../../include/stage_types.hh ../../include/truthserver.hh
../truthserver.o: ../../include/guiexport.hh
../visionbeacon.o: ../../include/world.hh ../../include/image.h
../visionbeacon.o: ../../include/entity.hh
../visionbeacon.o: /usr/local/player-1.0a/include/stage.h
../visionbeacon.o: /usr/local/player-1.0a/include/messages.h
../visionbeacon.o: /usr/local/player-1.0a/include/playercommon.h
../visionbeacon.o: ../../include/stage_types.hh ../../include/truthserver.hh
../visionbeacon.o: ../../include/guiexport.hh ../../include/visionbeacon.hh
../visiondevice.o: ../../include/world.hh ../../include/image.h
../visiondevice.o: ../../include/entity.hh
../visiondevice.o: /usr/local/player-1.0a/include/stage.h
../visiondevice.o: /usr/local/player-1.0a/include/messages.h
../visiondevice.o: /usr/local/player-1.0a/include/playercommon.h
../visiondevice.o: ../../include/stage_types.hh ../../include/truthserver.hh
../visiondevice.o: ../../include/guiexport.hh ../../include/visiondevice.hh
../visiondevice.o: ../../include/ptzdevice.hh ../../include/playerdevice.hh
../world.o: ../../include/world.hh ../../include/image.h
../world.o: ../../include/entity.hh /usr/local/player-1.0a/include/stage.h
../world.o: /usr/local/player-1.0a/include/messages.h
../world.o: /usr/local/player-1.0a/include/playercommon.h
../world.o: ../../include/stage_types.hh ../../include/truthserver.hh
../world.o: ../../include/guiexport.hh
../world_load.o: ../../include/stage_types.hh ../../include/world.hh
../world_load.o: ../../include/image.h ../../include/entity.hh
../world_load.o: /usr/local/player-1.0a/include/stage.h
../world_load.o: /usr/local/player-1.0a/include/messages.h
../world_load.o: /usr/local/player-1.0a/include/playercommon.h
../world_load.o: ../../include/truthserver.hh ../../include/guiexport.hh
../world_load.o: ../../include/entityfactory.hh
../xs.o: ../../include/stage_types.hh ../../include/truthserver.hh
../xs.o: /usr/local/player-1.0a/include/messages.h
../xs.o: /usr/local/player-1.0a/include/playercommon.h ../../include/xs.hh
