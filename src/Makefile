##############################################################################
#
# stage/src/Makefile - compiles all stage targets
# Author: Richard Vaughan, Andrew Howard
#
# $Id: Makefile,v 1.109 2002-07-09 02:59:51 rtv Exp $
#
# Note: All normal user configurations are in ../Makefile.common - you
# probably don't need to change this file
#
##############################################################################

# include the configuration file
include ../Makefile.common

# locate the player sources and libraries
PLAYER_INC = $(PLAYER_DIR)/include
PLAYER_LIB = $(PLAYER_DIR)/lib

# the includes everyone needs to have
INCLUDES = -I../include -I$(PLAYER_INC) 

# compile flags for Stage
CCFLAGS += $(ZLIB_FLAGS) $(INCLUDES) \
	-DVERSION=\"$(VERSION)\" -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\" \
	-DINCLUDE_RTK2 -I../$(RTK_INC) `gtk-config --cflags`\
	$(HRL_FLAGS) 

# Link flags for Stage
# optional libraries are linked if they were defined in Makefile.common
LINKFLAGS = -lm -lstdc++ -L$(PLAYER_DIR)/lib -lplayerqueue\
             $(ZLIB_LINK) $(PROFILE_LINK) $(HRL_LINK) $(OLDLIBS) \
	     -L../$(RTK_LIB) -lrtk `gtk-config --libs gtk gthread`

OBS = main.o world.o stageio.o server.o client.o worldfile.o \
	colors.o raytrace.o matrix.o image.o rtp.o entity.o entityfactory.o \
	playerdevice.o fixedobstacle.o 	positiondevice.o \
	omnipositiondevice.o laserdevice.o sonardevice.o \
	miscdevice.o broadcastdevice.o ptzdevice.o boxobstacle.o \
	laserbeacon.o visiondevice.o visionbeacon.o \
	puck.o gpsdevice.o gripperdevice.o motedevice.o bpsdevice.o \
	laserbeacondevice.o truthdevice.o descartesdevice.o irdevice.o

all: stage

# STAGE ####################################################################
RTKFILES = $(shell ls -d ../$(RTK_DIR)/*.[ch])
stage: $(RTKFILES) $(OBS)
	cd ../$(RTK_DIR) && ${MAKE}
	${LINKER} $(OBS) $(LINKFLAGS) -o $@

# SRC TO OBJECT RULES #######################################################
%.o : %.cc
	${CC} ${CCFLAGS} -c $< -o $@

# ADMIN #######################################################################
clean:
	rm -f *.o *~ *.a stage core core.*

# makedepend.  use it; it's your friend.  
dep:
	$(MAKEDEP) -a -o.o -Y $(INCLUDES) -s "# Dependancies (make dep)" *.cc 2> /dev/null
	rm -f Makefile.bak

clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak

# Dependancies (make dep)

boxobstacle.o: ../include/world.hh
boxobstacle.o: /home/vaughan/player-1.2/include/messages.h
boxobstacle.o: /home/vaughan/player-1.2/include/playercommon.h
boxobstacle.o: /home/vaughan/player-1.2/include/defaults.h
boxobstacle.o: ../include/image.hh ../include/entity.hh
boxobstacle.o: /home/vaughan/player-1.2/include/stage.h
boxobstacle.o: /home/vaughan/player-1.2/include/playerqueue.h
boxobstacle.o: ../include/stage_types.hh
boxobstacle.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
boxobstacle.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
boxobstacle.o: ../include/boxobstacle.hh
bpsdevice.o: /home/vaughan/player-1.2/include/stage.h
bpsdevice.o: /home/vaughan/player-1.2/include/messages.h
bpsdevice.o: /home/vaughan/player-1.2/include/playercommon.h
bpsdevice.o: /home/vaughan/player-1.2/include/defaults.h
bpsdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
bpsdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
bpsdevice.o: ../include/stage_types.hh
bpsdevice.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
bpsdevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
bpsdevice.o: ../include/bpsdevice.hh ../include/playerdevice.hh
broadcastdevice.o: /home/vaughan/player-1.2/include/stage.h
broadcastdevice.o: /home/vaughan/player-1.2/include/messages.h
broadcastdevice.o: /home/vaughan/player-1.2/include/playercommon.h
broadcastdevice.o: /home/vaughan/player-1.2/include/defaults.h
broadcastdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
broadcastdevice.o: ../include/world.hh ../include/image.hh
broadcastdevice.o: ../include/entity.hh ../include/stage_types.hh
broadcastdevice.o: /home/vaughan/player-1.2/include/player.h
broadcastdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
broadcastdevice.o: ../include/worldfile.hh ../include/broadcastdevice.hh
broadcastdevice.o: ../include/playerdevice.hh
client.o: ../include/server.hh /home/vaughan/player-1.2/include/messages.h
client.o: /home/vaughan/player-1.2/include/playercommon.h
client.o: /home/vaughan/player-1.2/include/defaults.h
client.o: ../include/stage_types.hh /home/vaughan/player-1.2/include/player.h
client.o: ../include/worldfile.hh ../include/world.hh ../include/image.hh
client.o: ../include/entity.hh /home/vaughan/player-1.2/include/stage.h
client.o: /home/vaughan/player-1.2/include/playerqueue.h ../include/colors.hh
client.o: ../include/rtp.h ../include/matrix.hh
colors.o: ../include/stage_types.hh /home/vaughan/player-1.2/include/player.h
colors.o: /home/vaughan/player-1.2/include/messages.h
colors.o: /home/vaughan/player-1.2/include/playercommon.h
colors.o: /home/vaughan/player-1.2/include/defaults.h ../include/colors.hh
descartesdevice.o: ../include/world.hh
descartesdevice.o: /home/vaughan/player-1.2/include/messages.h
descartesdevice.o: /home/vaughan/player-1.2/include/playercommon.h
descartesdevice.o: /home/vaughan/player-1.2/include/defaults.h
descartesdevice.o: ../include/image.hh ../include/entity.hh
descartesdevice.o: /home/vaughan/player-1.2/include/stage.h
descartesdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
descartesdevice.o: ../include/stage_types.hh
descartesdevice.o: /home/vaughan/player-1.2/include/player.h
descartesdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
descartesdevice.o: ../include/worldfile.hh ../include/descartesdevice.hh
descartesdevice.o: ../include/positiondevice.hh ../include/raytrace.hh
entity.o: ../include/entity.hh /home/vaughan/player-1.2/include/stage.h
entity.o: /home/vaughan/player-1.2/include/messages.h
entity.o: /home/vaughan/player-1.2/include/playercommon.h
entity.o: /home/vaughan/player-1.2/include/defaults.h
entity.o: /home/vaughan/player-1.2/include/playerqueue.h
entity.o: ../include/stage_types.hh /home/vaughan/player-1.2/include/player.h
entity.o: ../include/colors.hh ../include/rtp.h ../include/raytrace.hh
entity.o: ../include/matrix.hh ../include/image.hh ../include/world.hh
entity.o: ../include/worldfile.hh
entityfactory.o: ../include/entityfactory.hh ../include/fixedobstacle.hh
entityfactory.o: ../include/entity.hh
entityfactory.o: /home/vaughan/player-1.2/include/stage.h
entityfactory.o: /home/vaughan/player-1.2/include/messages.h
entityfactory.o: /home/vaughan/player-1.2/include/playercommon.h
entityfactory.o: /home/vaughan/player-1.2/include/defaults.h
entityfactory.o: /home/vaughan/player-1.2/include/playerqueue.h
entityfactory.o: ../include/stage_types.hh
entityfactory.o: /home/vaughan/player-1.2/include/player.h
entityfactory.o: ../include/colors.hh ../include/rtp.h
entityfactory.o: ../include/boxobstacle.hh ../include/laserbeacon.hh
entityfactory.o: ../include/puck.hh ../include/visionbeacon.hh
entityfactory.o: ../include/sonardevice.hh ../include/playerdevice.hh
entityfactory.o: ../include/positiondevice.hh
entityfactory.o: ../include/omnipositiondevice.hh ../include/laserdevice.hh
entityfactory.o: ../include/miscdevice.hh ../include/ptzdevice.hh
entityfactory.o: ../include/visiondevice.hh ../include/laserbeacondevice.hh
entityfactory.o: ../include/guiexport.hh ../include/broadcastdevice.hh
entityfactory.o: ../include/bpsdevice.hh ../include/gripperdevice.hh
entityfactory.o: ../include/gpsdevice.hh ../include/motedevice.hh
entityfactory.o: ../include/truthdevice.hh ../include/irdevice.hh
entityfactory.o: ../include/descartesdevice.hh ../include/world.hh
entityfactory.o: ../include/image.hh ../include/matrix.hh
entityfactory.o: ../include/worldfile.hh
envserver.o: ../include/world.hh /home/vaughan/player-1.2/include/messages.h
envserver.o: /home/vaughan/player-1.2/include/playercommon.h
envserver.o: /home/vaughan/player-1.2/include/defaults.h ../include/image.hh
envserver.o: ../include/entity.hh /home/vaughan/player-1.2/include/stage.h
envserver.o: /home/vaughan/player-1.2/include/playerqueue.h
envserver.o: ../include/stage_types.hh
envserver.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
envserver.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
envserver.o: ../include/truthserver.hh
fixedobstacle.o: ../include/image.hh ../include/world.hh
fixedobstacle.o: /home/vaughan/player-1.2/include/messages.h
fixedobstacle.o: /home/vaughan/player-1.2/include/playercommon.h
fixedobstacle.o: /home/vaughan/player-1.2/include/defaults.h
fixedobstacle.o: ../include/entity.hh
fixedobstacle.o: /home/vaughan/player-1.2/include/stage.h
fixedobstacle.o: /home/vaughan/player-1.2/include/playerqueue.h
fixedobstacle.o: ../include/stage_types.hh
fixedobstacle.o: /home/vaughan/player-1.2/include/player.h
fixedobstacle.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
fixedobstacle.o: ../include/worldfile.hh ../include/fixedobstacle.hh
gpsdevice.o: /home/vaughan/player-1.2/include/stage.h
gpsdevice.o: /home/vaughan/player-1.2/include/messages.h
gpsdevice.o: /home/vaughan/player-1.2/include/playercommon.h
gpsdevice.o: /home/vaughan/player-1.2/include/defaults.h
gpsdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
gpsdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
gpsdevice.o: ../include/stage_types.hh
gpsdevice.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
gpsdevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
gpsdevice.o: ../include/gpsdevice.hh
gripperdevice.o: ../include/world.hh
gripperdevice.o: /home/vaughan/player-1.2/include/messages.h
gripperdevice.o: /home/vaughan/player-1.2/include/playercommon.h
gripperdevice.o: /home/vaughan/player-1.2/include/defaults.h
gripperdevice.o: ../include/image.hh ../include/entity.hh
gripperdevice.o: /home/vaughan/player-1.2/include/stage.h
gripperdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
gripperdevice.o: ../include/stage_types.hh
gripperdevice.o: /home/vaughan/player-1.2/include/player.h
gripperdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
gripperdevice.o: ../include/worldfile.hh ../include/gripperdevice.hh
gripperdevice.o: ../include/raytrace.hh
image.o: ../include/stage_types.hh /home/vaughan/player-1.2/include/player.h
image.o: /home/vaughan/player-1.2/include/messages.h
image.o: /home/vaughan/player-1.2/include/playercommon.h
image.o: /home/vaughan/player-1.2/include/defaults.h ../include/image.hh
irdevice.o: ../include/world.hh /home/vaughan/player-1.2/include/messages.h
irdevice.o: /home/vaughan/player-1.2/include/playercommon.h
irdevice.o: /home/vaughan/player-1.2/include/defaults.h ../include/image.hh
irdevice.o: ../include/entity.hh /home/vaughan/player-1.2/include/stage.h
irdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
irdevice.o: ../include/stage_types.hh
irdevice.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
irdevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
irdevice.o: ../include/irdevice.hh ../include/raytrace.hh
laserbeacon.o: ../include/world.hh
laserbeacon.o: /home/vaughan/player-1.2/include/messages.h
laserbeacon.o: /home/vaughan/player-1.2/include/playercommon.h
laserbeacon.o: /home/vaughan/player-1.2/include/defaults.h
laserbeacon.o: ../include/image.hh ../include/entity.hh
laserbeacon.o: /home/vaughan/player-1.2/include/stage.h
laserbeacon.o: /home/vaughan/player-1.2/include/playerqueue.h
laserbeacon.o: ../include/stage_types.hh
laserbeacon.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
laserbeacon.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
laserbeacon.o: ../include/laserbeacon.hh ../include/laserdevice.hh
laserbeacon.o: ../include/playerdevice.hh
laserbeacondevice.o: /home/vaughan/player-1.2/include/stage.h
laserbeacondevice.o: /home/vaughan/player-1.2/include/messages.h
laserbeacondevice.o: /home/vaughan/player-1.2/include/playercommon.h
laserbeacondevice.o: /home/vaughan/player-1.2/include/defaults.h
laserbeacondevice.o: /home/vaughan/player-1.2/include/playerqueue.h
laserbeacondevice.o: ../include/world.hh ../include/image.hh
laserbeacondevice.o: ../include/entity.hh ../include/stage_types.hh
laserbeacondevice.o: /home/vaughan/player-1.2/include/player.h
laserbeacondevice.o: ../include/colors.hh ../include/rtp.h
laserbeacondevice.o: ../include/matrix.hh ../include/worldfile.hh
laserbeacondevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacon.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh ../include/guiexport.hh
laserdevice.o: /home/vaughan/player-1.2/include/stage.h
laserdevice.o: /home/vaughan/player-1.2/include/messages.h
laserdevice.o: /home/vaughan/player-1.2/include/playercommon.h
laserdevice.o: /home/vaughan/player-1.2/include/defaults.h
laserdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
laserdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
laserdevice.o: ../include/stage_types.hh
laserdevice.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
laserdevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
laserdevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserdevice.o: ../include/laserbeacon.hh ../include/raytrace.hh
main.o: ../include/server.hh /home/vaughan/player-1.2/include/messages.h
main.o: /home/vaughan/player-1.2/include/playercommon.h
main.o: /home/vaughan/player-1.2/include/defaults.h ../include/stage_types.hh
main.o: /home/vaughan/player-1.2/include/player.h ../include/worldfile.hh
main.o: ../include/world.hh ../include/image.hh ../include/entity.hh
main.o: /home/vaughan/player-1.2/include/stage.h
main.o: /home/vaughan/player-1.2/include/playerqueue.h ../include/colors.hh
main.o: ../include/rtp.h ../include/matrix.hh
manager.o: ../include/truthserver.hh
manager.o: /home/vaughan/player-1.2/include/messages.h
manager.o: /home/vaughan/player-1.2/include/playercommon.h
manager.o: /home/vaughan/player-1.2/include/defaults.h
manager.o: ../include/stage_types.hh
manager.o: /home/vaughan/player-1.2/include/player.h
matrix.o: ../include/matrix.hh ../include/image.hh ../include/entity.hh
matrix.o: /home/vaughan/player-1.2/include/stage.h
matrix.o: /home/vaughan/player-1.2/include/messages.h
matrix.o: /home/vaughan/player-1.2/include/playercommon.h
matrix.o: /home/vaughan/player-1.2/include/defaults.h
matrix.o: /home/vaughan/player-1.2/include/playerqueue.h
matrix.o: ../include/stage_types.hh /home/vaughan/player-1.2/include/player.h
matrix.o: ../include/colors.hh ../include/rtp.h
miscdevice.o: /home/vaughan/player-1.2/include/stage.h
miscdevice.o: /home/vaughan/player-1.2/include/messages.h
miscdevice.o: /home/vaughan/player-1.2/include/playercommon.h
miscdevice.o: /home/vaughan/player-1.2/include/defaults.h
miscdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
miscdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
miscdevice.o: ../include/stage_types.hh
miscdevice.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
miscdevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
miscdevice.o: ../include/miscdevice.hh ../include/playerdevice.hh
motedevice.o: /home/vaughan/player-1.2/include/stage.h
motedevice.o: /home/vaughan/player-1.2/include/messages.h
motedevice.o: /home/vaughan/player-1.2/include/playercommon.h
motedevice.o: /home/vaughan/player-1.2/include/defaults.h
motedevice.o: /home/vaughan/player-1.2/include/playerqueue.h
motedevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
motedevice.o: ../include/stage_types.hh
motedevice.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
motedevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
motedevice.o: ../include/motedevice.hh
omnipositiondevice.o: ../include/world.hh
omnipositiondevice.o: /home/vaughan/player-1.2/include/messages.h
omnipositiondevice.o: /home/vaughan/player-1.2/include/playercommon.h
omnipositiondevice.o: /home/vaughan/player-1.2/include/defaults.h
omnipositiondevice.o: ../include/image.hh ../include/entity.hh
omnipositiondevice.o: /home/vaughan/player-1.2/include/stage.h
omnipositiondevice.o: /home/vaughan/player-1.2/include/playerqueue.h
omnipositiondevice.o: ../include/stage_types.hh
omnipositiondevice.o: /home/vaughan/player-1.2/include/player.h
omnipositiondevice.o: ../include/colors.hh ../include/rtp.h
omnipositiondevice.o: ../include/matrix.hh ../include/worldfile.hh
omnipositiondevice.o: ../include/raytrace.hh ../include/omnipositiondevice.hh
playerdevice.o: /home/vaughan/player-1.2/include/stage.h
playerdevice.o: /home/vaughan/player-1.2/include/messages.h
playerdevice.o: /home/vaughan/player-1.2/include/playercommon.h
playerdevice.o: /home/vaughan/player-1.2/include/defaults.h
playerdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
playerdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
playerdevice.o: ../include/stage_types.hh
playerdevice.o: /home/vaughan/player-1.2/include/player.h
playerdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
playerdevice.o: ../include/worldfile.hh ../include/playerdevice.hh
positiondevice.o: ../include/world.hh
positiondevice.o: /home/vaughan/player-1.2/include/messages.h
positiondevice.o: /home/vaughan/player-1.2/include/playercommon.h
positiondevice.o: /home/vaughan/player-1.2/include/defaults.h
positiondevice.o: ../include/image.hh ../include/entity.hh
positiondevice.o: /home/vaughan/player-1.2/include/stage.h
positiondevice.o: /home/vaughan/player-1.2/include/playerqueue.h
positiondevice.o: ../include/stage_types.hh
positiondevice.o: /home/vaughan/player-1.2/include/player.h
positiondevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
positiondevice.o: ../include/worldfile.hh ../include/positiondevice.hh
ptzdevice.o: ../include/world.hh /home/vaughan/player-1.2/include/messages.h
ptzdevice.o: /home/vaughan/player-1.2/include/playercommon.h
ptzdevice.o: /home/vaughan/player-1.2/include/defaults.h ../include/image.hh
ptzdevice.o: ../include/entity.hh /home/vaughan/player-1.2/include/stage.h
ptzdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
ptzdevice.o: ../include/stage_types.hh
ptzdevice.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
ptzdevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
ptzdevice.o: ../include/ptzdevice.hh ../include/playerdevice.hh
puck.o: ../include/world.hh /home/vaughan/player-1.2/include/messages.h
puck.o: /home/vaughan/player-1.2/include/playercommon.h
puck.o: /home/vaughan/player-1.2/include/defaults.h ../include/image.hh
puck.o: ../include/entity.hh /home/vaughan/player-1.2/include/stage.h
puck.o: /home/vaughan/player-1.2/include/playerqueue.h
puck.o: ../include/stage_types.hh /home/vaughan/player-1.2/include/player.h
puck.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
puck.o: ../include/worldfile.hh ../include/puck.hh ../include/raytrace.hh
raytrace.o: ../include/raytrace.hh ../include/matrix.hh ../include/image.hh
raytrace.o: ../include/entity.hh /home/vaughan/player-1.2/include/stage.h
raytrace.o: /home/vaughan/player-1.2/include/messages.h
raytrace.o: /home/vaughan/player-1.2/include/playercommon.h
raytrace.o: /home/vaughan/player-1.2/include/defaults.h
raytrace.o: /home/vaughan/player-1.2/include/playerqueue.h
raytrace.o: ../include/stage_types.hh
raytrace.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
raytrace.o: ../include/rtp.h ../include/world.hh ../include/worldfile.hh
rtp.o: ../include/rtp.h
server.o: ../include/server.hh /home/vaughan/player-1.2/include/messages.h
server.o: /home/vaughan/player-1.2/include/playercommon.h
server.o: /home/vaughan/player-1.2/include/defaults.h
server.o: ../include/stage_types.hh /home/vaughan/player-1.2/include/player.h
server.o: ../include/worldfile.hh ../include/world.hh ../include/image.hh
server.o: ../include/entity.hh /home/vaughan/player-1.2/include/stage.h
server.o: /home/vaughan/player-1.2/include/playerqueue.h ../include/colors.hh
server.o: ../include/rtp.h ../include/matrix.hh ../include/fixedobstacle.hh
sonardevice.o: ../include/world.hh
sonardevice.o: /home/vaughan/player-1.2/include/messages.h
sonardevice.o: /home/vaughan/player-1.2/include/playercommon.h
sonardevice.o: /home/vaughan/player-1.2/include/defaults.h
sonardevice.o: ../include/image.hh ../include/entity.hh
sonardevice.o: /home/vaughan/player-1.2/include/stage.h
sonardevice.o: /home/vaughan/player-1.2/include/playerqueue.h
sonardevice.o: ../include/stage_types.hh
sonardevice.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
sonardevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
sonardevice.o: ../include/sonardevice.hh ../include/playerdevice.hh
sonardevice.o: ../include/raytrace.hh
stageio.o: ../include/server.hh /home/vaughan/player-1.2/include/messages.h
stageio.o: /home/vaughan/player-1.2/include/playercommon.h
stageio.o: /home/vaughan/player-1.2/include/defaults.h
stageio.o: ../include/stage_types.hh
stageio.o: /home/vaughan/player-1.2/include/player.h ../include/worldfile.hh
stageio.o: ../include/world.hh ../include/image.hh ../include/entity.hh
stageio.o: /home/vaughan/player-1.2/include/stage.h
stageio.o: /home/vaughan/player-1.2/include/playerqueue.h
stageio.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
stageio.o: ../include/fixedobstacle.hh
truthdevice.o: ../include/world.hh
truthdevice.o: /home/vaughan/player-1.2/include/messages.h
truthdevice.o: /home/vaughan/player-1.2/include/playercommon.h
truthdevice.o: /home/vaughan/player-1.2/include/defaults.h
truthdevice.o: ../include/image.hh ../include/entity.hh
truthdevice.o: /home/vaughan/player-1.2/include/stage.h
truthdevice.o: /home/vaughan/player-1.2/include/playerqueue.h
truthdevice.o: ../include/stage_types.hh
truthdevice.o: /home/vaughan/player-1.2/include/player.h ../include/colors.hh
truthdevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
truthdevice.o: ../include/truthdevice.hh
truthlog.o: ../include/truthserver.hh
truthlog.o: /home/vaughan/player-1.2/include/messages.h
truthlog.o: /home/vaughan/player-1.2/include/playercommon.h
truthlog.o: /home/vaughan/player-1.2/include/defaults.h
truthlog.o: ../include/stage_types.hh
truthlog.o: /home/vaughan/player-1.2/include/player.h
visionbeacon.o: ../include/world.hh
visionbeacon.o: /home/vaughan/player-1.2/include/messages.h
visionbeacon.o: /home/vaughan/player-1.2/include/playercommon.h
visionbeacon.o: /home/vaughan/player-1.2/include/defaults.h
visionbeacon.o: ../include/image.hh ../include/entity.hh
visionbeacon.o: /home/vaughan/player-1.2/include/stage.h
visionbeacon.o: /home/vaughan/player-1.2/include/playerqueue.h
visionbeacon.o: ../include/stage_types.hh
visionbeacon.o: /home/vaughan/player-1.2/include/player.h
visionbeacon.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
visionbeacon.o: ../include/worldfile.hh ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh
visiondevice.o: /home/vaughan/player-1.2/include/messages.h
visiondevice.o: /home/vaughan/player-1.2/include/playercommon.h
visiondevice.o: /home/vaughan/player-1.2/include/defaults.h
visiondevice.o: ../include/image.hh ../include/entity.hh
visiondevice.o: /home/vaughan/player-1.2/include/stage.h
visiondevice.o: /home/vaughan/player-1.2/include/playerqueue.h
visiondevice.o: ../include/stage_types.hh
visiondevice.o: /home/vaughan/player-1.2/include/player.h
visiondevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
visiondevice.o: ../include/worldfile.hh ../include/visiondevice.hh
visiondevice.o: ../include/ptzdevice.hh ../include/playerdevice.hh
visiondevice.o: ../include/raytrace.hh
world.o: ../include/world.hh /home/vaughan/player-1.2/include/messages.h
world.o: /home/vaughan/player-1.2/include/playercommon.h
world.o: /home/vaughan/player-1.2/include/defaults.h ../include/image.hh
world.o: ../include/entity.hh /home/vaughan/player-1.2/include/stage.h
world.o: /home/vaughan/player-1.2/include/playerqueue.h
world.o: ../include/stage_types.hh /home/vaughan/player-1.2/include/player.h
world.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
world.o: ../include/worldfile.hh ../include/fixedobstacle.hh
worldfile.o: ../include/stage_types.hh
worldfile.o: /home/vaughan/player-1.2/include/player.h
worldfile.o: /home/vaughan/player-1.2/include/messages.h
worldfile.o: /home/vaughan/player-1.2/include/playercommon.h
worldfile.o: /home/vaughan/player-1.2/include/defaults.h ../include/colors.hh
worldfile.o: ../include/worldfile.hh
