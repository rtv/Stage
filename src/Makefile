##############################################################################
#
# stage/src/Makefile - compiles all stage targets
# Author: Richard Vaughan, Andrew Howard
#
# $Id: Makefile,v 1.94 2002-06-08 20:52:54 inspectorg Exp $
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
STAGEFLAGS = -DREENTRANT $(ZLIB_FLAGS) $(INCLUDES) $(PLAYER_PLATFORM) \
             -DVERSION=\"$(VERSION)\" -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\" \
				     -DINCLUDE_RTK2 -I../$(RTK_INC) `gtk-config --cflags`\
	   				  $(HRL_FLAGS) $(CCFLAGS)	

# Link flags for Stage
# optional libraries are linked if they were defined in Makefile.common
STAGELINK =  -lm -lstdc++ -pthread -L$(PLAYER_DIR)/lib -lplayerqueue\
             $(ZLIB_LINK) $(PROFILE_LINK) $(HRL_LINK) $(OLDLIBS) \
	     -L../$(RTK_LIB) -lrtk `gtk-config --libs gtk gthread`

OBS = stageio.o server.o client.o entity.o main.o world.o worldfile.o \
	playerdevice.o fixedobstacle.o 	positiondevice.o \
	omnipositiondevice.o laserdevice.o sonardevice.o \
	miscdevice.o broadcastdevice.o ptzdevice.o boxobstacle.o \
	laserbeacon.o visiondevice.o visionbeacon.o \
	puck.o gpsdevice.o gripperdevice.o motedevice.o bpsdevice.o \
	laserbeacondevice.o truthdevice.o \
	raytrace.o matrix.o image.o rtp.o entityfactory.o 

# TARGETS  #########################################################
all: stage #tools 

# STAGE ####################################################################
stage: rtk $(OBS)
	$(LINKER) $(OBS) $(STAGELINK) -o $@

# RTK ######################################################################
rtk:
	cd ../$(RTK_DIR) && ${MAKE}



# TOOLS ###################################################################

tools: manager truthlog trapdoor

manager: manager.cc 
	$(CC) -Wall $(INCLUDES) -lstdc++ manager.cc -o manager

# truthlog doesn't compile... BPG - 'cos it's broken... RTV
truthlog: truthlog.cc 
	$(CC) $(INCLUDES) -lstdc++ truthlog.cc -o truthlog

trapdoor: libtrapdoor.a

libtrapdoor.a: trapdoor.cc
	${CC} ${CCFLAGS} ${INCLUDES} -c $< -o $@
	ar cr $@ trapdoor.o && ranlib $@  

# SRC TO OBJECT RULES #######################################################

%.o : %.cc
	${CC} ${STAGEFLAGS} -c $< -o $@

# ADMIN #######################################################################

clean:
	rm -f *.o *~ *.a stage manager core core.* xs truthlog 

# makedepend.  use it; it's your friend.  here we run 3 times in order to
# derive the dependencies for the *_stg.o objects, the *_rtk.o objects, and 
# the *.o objects for things like xs.  some unnecessary and unused 
# dependencies will be added (e.g., world.o), but they're harmless - BPG
dep:
	$(MAKEDEP) -a -o.o -Y $(INCLUDES) -s "# Dependancies (make dep)" *.cc 2> /dev/null
	rm -f Makefile.bak

clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak

# Dependancies (make dep)

boxobstacle.o: ../include/world.hh
boxobstacle.o: /home/ahoward/player-1.2-alpha/include/messages.h
boxobstacle.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
boxobstacle.o: /home/ahoward/player-1.2-alpha/include/defaults.h
boxobstacle.o: ../include/image.hh ../include/entity.hh
boxobstacle.o: /home/ahoward/player-1.2-alpha/include/stage.h
boxobstacle.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
boxobstacle.o: ../include/stage_types.hh
boxobstacle.o: /home/ahoward/player-1.2-alpha/include/player.h
boxobstacle.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
boxobstacle.o: ../include/worldfile.hh ../include/boxobstacle.hh
bpsdevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
bpsdevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
bpsdevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
bpsdevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
bpsdevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
bpsdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
bpsdevice.o: ../include/stage_types.hh
bpsdevice.o: /home/ahoward/player-1.2-alpha/include/player.h
bpsdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
bpsdevice.o: ../include/worldfile.hh ../include/bpsdevice.hh
bpsdevice.o: ../include/playerdevice.hh
broadcastdevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
broadcastdevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
broadcastdevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
broadcastdevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
broadcastdevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
broadcastdevice.o: ../include/world.hh ../include/image.hh
broadcastdevice.o: ../include/entity.hh ../include/stage_types.hh
broadcastdevice.o: /home/ahoward/player-1.2-alpha/include/player.h
broadcastdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
broadcastdevice.o: ../include/worldfile.hh ../include/broadcastdevice.hh
broadcastdevice.o: ../include/playerdevice.hh
client.o: ../include/server.hh
client.o: /home/ahoward/player-1.2-alpha/include/messages.h
client.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
client.o: /home/ahoward/player-1.2-alpha/include/defaults.h
client.o: ../include/stage_types.hh
client.o: /home/ahoward/player-1.2-alpha/include/player.h
client.o: ../include/worldfile.hh ../include/world.hh ../include/image.hh
client.o: ../include/entity.hh /home/ahoward/player-1.2-alpha/include/stage.h
client.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
client.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
entity.o: ../include/entity.hh /home/ahoward/player-1.2-alpha/include/stage.h
entity.o: /home/ahoward/player-1.2-alpha/include/messages.h
entity.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
entity.o: /home/ahoward/player-1.2-alpha/include/defaults.h
entity.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
entity.o: ../include/stage_types.hh
entity.o: /home/ahoward/player-1.2-alpha/include/player.h
entity.o: ../include/colors.hh ../include/rtp.h ../include/raytrace.hh
entity.o: ../include/matrix.hh ../include/image.hh ../include/world.hh
entity.o: ../include/worldfile.hh
entityfactory.o: ../include/entityfactory.hh ../include/fixedobstacle.hh
entityfactory.o: ../include/entity.hh
entityfactory.o: /home/ahoward/player-1.2-alpha/include/stage.h
entityfactory.o: /home/ahoward/player-1.2-alpha/include/messages.h
entityfactory.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
entityfactory.o: /home/ahoward/player-1.2-alpha/include/defaults.h
entityfactory.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
entityfactory.o: ../include/stage_types.hh
entityfactory.o: /home/ahoward/player-1.2-alpha/include/player.h
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
entityfactory.o: ../include/truthdevice.hh ../include/world.hh
entityfactory.o: ../include/image.hh ../include/matrix.hh
entityfactory.o: ../include/worldfile.hh
envserver.o: ../include/world.hh
envserver.o: /home/ahoward/player-1.2-alpha/include/messages.h
envserver.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
envserver.o: /home/ahoward/player-1.2-alpha/include/defaults.h
envserver.o: ../include/image.hh ../include/entity.hh
envserver.o: /home/ahoward/player-1.2-alpha/include/stage.h
envserver.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
envserver.o: ../include/stage_types.hh
envserver.o: /home/ahoward/player-1.2-alpha/include/player.h
envserver.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
envserver.o: ../include/worldfile.hh ../include/truthserver.hh
fixedobstacle.o: ../include/image.hh ../include/world.hh
fixedobstacle.o: /home/ahoward/player-1.2-alpha/include/messages.h
fixedobstacle.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
fixedobstacle.o: /home/ahoward/player-1.2-alpha/include/defaults.h
fixedobstacle.o: ../include/entity.hh
fixedobstacle.o: /home/ahoward/player-1.2-alpha/include/stage.h
fixedobstacle.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
fixedobstacle.o: ../include/stage_types.hh
fixedobstacle.o: /home/ahoward/player-1.2-alpha/include/player.h
fixedobstacle.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
fixedobstacle.o: ../include/worldfile.hh ../include/fixedobstacle.hh
gpsdevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
gpsdevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
gpsdevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
gpsdevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
gpsdevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
gpsdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
gpsdevice.o: ../include/stage_types.hh
gpsdevice.o: /home/ahoward/player-1.2-alpha/include/player.h
gpsdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
gpsdevice.o: ../include/worldfile.hh ../include/gpsdevice.hh
gripperdevice.o: ../include/world.hh
gripperdevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
gripperdevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
gripperdevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
gripperdevice.o: ../include/image.hh ../include/entity.hh
gripperdevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
gripperdevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
gripperdevice.o: ../include/stage_types.hh
gripperdevice.o: /home/ahoward/player-1.2-alpha/include/player.h
gripperdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
gripperdevice.o: ../include/worldfile.hh ../include/gripperdevice.hh
gripperdevice.o: ../include/raytrace.hh
image.o: ../include/image.hh
laserbeacon.o: ../include/world.hh
laserbeacon.o: /home/ahoward/player-1.2-alpha/include/messages.h
laserbeacon.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
laserbeacon.o: /home/ahoward/player-1.2-alpha/include/defaults.h
laserbeacon.o: ../include/image.hh ../include/entity.hh
laserbeacon.o: /home/ahoward/player-1.2-alpha/include/stage.h
laserbeacon.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
laserbeacon.o: ../include/stage_types.hh
laserbeacon.o: /home/ahoward/player-1.2-alpha/include/player.h
laserbeacon.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
laserbeacon.o: ../include/worldfile.hh ../include/laserbeacon.hh
laserbeacon.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserbeacondevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
laserbeacondevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
laserbeacondevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
laserbeacondevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
laserbeacondevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
laserbeacondevice.o: ../include/world.hh ../include/image.hh
laserbeacondevice.o: ../include/entity.hh ../include/stage_types.hh
laserbeacondevice.o: /home/ahoward/player-1.2-alpha/include/player.h
laserbeacondevice.o: ../include/colors.hh ../include/rtp.h
laserbeacondevice.o: ../include/matrix.hh ../include/worldfile.hh
laserbeacondevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacon.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh ../include/guiexport.hh
laserdevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
laserdevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
laserdevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
laserdevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
laserdevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
laserdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
laserdevice.o: ../include/stage_types.hh
laserdevice.o: /home/ahoward/player-1.2-alpha/include/player.h
laserdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
laserdevice.o: ../include/worldfile.hh ../include/laserdevice.hh
laserdevice.o: ../include/playerdevice.hh ../include/laserbeacon.hh
laserdevice.o: ../include/raytrace.hh
main.o: ../include/server.hh
main.o: /home/ahoward/player-1.2-alpha/include/messages.h
main.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
main.o: /home/ahoward/player-1.2-alpha/include/defaults.h
main.o: ../include/stage_types.hh
main.o: /home/ahoward/player-1.2-alpha/include/player.h
main.o: ../include/worldfile.hh ../include/world.hh ../include/image.hh
main.o: ../include/entity.hh /home/ahoward/player-1.2-alpha/include/stage.h
main.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
main.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
manager.o: ../include/truthserver.hh
manager.o: /home/ahoward/player-1.2-alpha/include/messages.h
manager.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
manager.o: /home/ahoward/player-1.2-alpha/include/defaults.h
manager.o: ../include/stage_types.hh
manager.o: /home/ahoward/player-1.2-alpha/include/player.h
matrix.o: ../include/matrix.hh ../include/image.hh ../include/entity.hh
matrix.o: /home/ahoward/player-1.2-alpha/include/stage.h
matrix.o: /home/ahoward/player-1.2-alpha/include/messages.h
matrix.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
matrix.o: /home/ahoward/player-1.2-alpha/include/defaults.h
matrix.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
matrix.o: ../include/stage_types.hh
matrix.o: /home/ahoward/player-1.2-alpha/include/player.h
matrix.o: ../include/colors.hh ../include/rtp.h
miscdevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
miscdevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
miscdevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
miscdevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
miscdevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
miscdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
miscdevice.o: ../include/stage_types.hh
miscdevice.o: /home/ahoward/player-1.2-alpha/include/player.h
miscdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
miscdevice.o: ../include/worldfile.hh ../include/miscdevice.hh
miscdevice.o: ../include/playerdevice.hh
motedevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
motedevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
motedevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
motedevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
motedevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
motedevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
motedevice.o: ../include/stage_types.hh
motedevice.o: /home/ahoward/player-1.2-alpha/include/player.h
motedevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
motedevice.o: ../include/worldfile.hh ../include/motedevice.hh
omnipositiondevice.o: ../include/world.hh
omnipositiondevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
omnipositiondevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
omnipositiondevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
omnipositiondevice.o: ../include/image.hh ../include/entity.hh
omnipositiondevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
omnipositiondevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
omnipositiondevice.o: ../include/stage_types.hh
omnipositiondevice.o: /home/ahoward/player-1.2-alpha/include/player.h
omnipositiondevice.o: ../include/colors.hh ../include/rtp.h
omnipositiondevice.o: ../include/matrix.hh ../include/worldfile.hh
omnipositiondevice.o: ../include/raytrace.hh ../include/omnipositiondevice.hh
playerdevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
playerdevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
playerdevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
playerdevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
playerdevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
playerdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
playerdevice.o: ../include/stage_types.hh
playerdevice.o: /home/ahoward/player-1.2-alpha/include/player.h
playerdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
playerdevice.o: ../include/worldfile.hh ../include/playerdevice.hh
positiondevice.o: ../include/world.hh
positiondevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
positiondevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
positiondevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
positiondevice.o: ../include/image.hh ../include/entity.hh
positiondevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
positiondevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
positiondevice.o: ../include/stage_types.hh
positiondevice.o: /home/ahoward/player-1.2-alpha/include/player.h
positiondevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
positiondevice.o: ../include/worldfile.hh ../include/positiondevice.hh
ptzdevice.o: ../include/world.hh
ptzdevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
ptzdevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
ptzdevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
ptzdevice.o: ../include/image.hh ../include/entity.hh
ptzdevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
ptzdevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
ptzdevice.o: ../include/stage_types.hh
ptzdevice.o: /home/ahoward/player-1.2-alpha/include/player.h
ptzdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
ptzdevice.o: ../include/worldfile.hh ../include/ptzdevice.hh
ptzdevice.o: ../include/playerdevice.hh
puck.o: ../include/world.hh /home/ahoward/player-1.2-alpha/include/messages.h
puck.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
puck.o: /home/ahoward/player-1.2-alpha/include/defaults.h ../include/image.hh
puck.o: ../include/entity.hh /home/ahoward/player-1.2-alpha/include/stage.h
puck.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
puck.o: ../include/stage_types.hh
puck.o: /home/ahoward/player-1.2-alpha/include/player.h ../include/colors.hh
puck.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
puck.o: ../include/puck.hh ../include/raytrace.hh
raytrace.o: ../include/raytrace.hh ../include/matrix.hh ../include/image.hh
raytrace.o: ../include/entity.hh
raytrace.o: /home/ahoward/player-1.2-alpha/include/stage.h
raytrace.o: /home/ahoward/player-1.2-alpha/include/messages.h
raytrace.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
raytrace.o: /home/ahoward/player-1.2-alpha/include/defaults.h
raytrace.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
raytrace.o: ../include/stage_types.hh
raytrace.o: /home/ahoward/player-1.2-alpha/include/player.h
raytrace.o: ../include/colors.hh ../include/rtp.h ../include/world.hh
raytrace.o: ../include/worldfile.hh
rtp.o: ../include/rtp.h
server.o: ../include/server.hh
server.o: /home/ahoward/player-1.2-alpha/include/messages.h
server.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
server.o: /home/ahoward/player-1.2-alpha/include/defaults.h
server.o: ../include/stage_types.hh
server.o: /home/ahoward/player-1.2-alpha/include/player.h
server.o: ../include/worldfile.hh ../include/world.hh ../include/image.hh
server.o: ../include/entity.hh /home/ahoward/player-1.2-alpha/include/stage.h
server.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
server.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
server.o: ../include/fixedobstacle.hh
sonardevice.o: ../include/world.hh
sonardevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
sonardevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
sonardevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
sonardevice.o: ../include/image.hh ../include/entity.hh
sonardevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
sonardevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
sonardevice.o: ../include/stage_types.hh
sonardevice.o: /home/ahoward/player-1.2-alpha/include/player.h
sonardevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
sonardevice.o: ../include/worldfile.hh ../include/sonardevice.hh
sonardevice.o: ../include/playerdevice.hh ../include/raytrace.hh
stageio.o: ../include/server.hh
stageio.o: /home/ahoward/player-1.2-alpha/include/messages.h
stageio.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
stageio.o: /home/ahoward/player-1.2-alpha/include/defaults.h
stageio.o: ../include/stage_types.hh
stageio.o: /home/ahoward/player-1.2-alpha/include/player.h
stageio.o: ../include/worldfile.hh ../include/world.hh ../include/image.hh
stageio.o: ../include/entity.hh
stageio.o: /home/ahoward/player-1.2-alpha/include/stage.h
stageio.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
stageio.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
stageio.o: ../include/fixedobstacle.hh
truthdevice.o: ../include/world.hh
truthdevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
truthdevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
truthdevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
truthdevice.o: ../include/image.hh ../include/entity.hh
truthdevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
truthdevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
truthdevice.o: ../include/stage_types.hh
truthdevice.o: /home/ahoward/player-1.2-alpha/include/player.h
truthdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
truthdevice.o: ../include/worldfile.hh ../include/truthdevice.hh
truthlog.o: ../include/truthserver.hh
truthlog.o: /home/ahoward/player-1.2-alpha/include/messages.h
truthlog.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
truthlog.o: /home/ahoward/player-1.2-alpha/include/defaults.h
truthlog.o: ../include/stage_types.hh
truthlog.o: /home/ahoward/player-1.2-alpha/include/player.h
visionbeacon.o: ../include/world.hh
visionbeacon.o: /home/ahoward/player-1.2-alpha/include/messages.h
visionbeacon.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
visionbeacon.o: /home/ahoward/player-1.2-alpha/include/defaults.h
visionbeacon.o: ../include/image.hh ../include/entity.hh
visionbeacon.o: /home/ahoward/player-1.2-alpha/include/stage.h
visionbeacon.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
visionbeacon.o: ../include/stage_types.hh
visionbeacon.o: /home/ahoward/player-1.2-alpha/include/player.h
visionbeacon.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
visionbeacon.o: ../include/worldfile.hh ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh
visiondevice.o: /home/ahoward/player-1.2-alpha/include/messages.h
visiondevice.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
visiondevice.o: /home/ahoward/player-1.2-alpha/include/defaults.h
visiondevice.o: ../include/image.hh ../include/entity.hh
visiondevice.o: /home/ahoward/player-1.2-alpha/include/stage.h
visiondevice.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
visiondevice.o: ../include/stage_types.hh
visiondevice.o: /home/ahoward/player-1.2-alpha/include/player.h
visiondevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
visiondevice.o: ../include/worldfile.hh ../include/visiondevice.hh
visiondevice.o: ../include/ptzdevice.hh ../include/playerdevice.hh
visiondevice.o: ../include/raytrace.hh
world.o: ../include/world.hh
world.o: /home/ahoward/player-1.2-alpha/include/messages.h
world.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
world.o: /home/ahoward/player-1.2-alpha/include/defaults.h
world.o: ../include/image.hh ../include/entity.hh
world.o: /home/ahoward/player-1.2-alpha/include/stage.h
world.o: /home/ahoward/player-1.2-alpha/include/playerqueue.h
world.o: ../include/stage_types.hh
world.o: /home/ahoward/player-1.2-alpha/include/player.h ../include/colors.hh
world.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
world.o: ../include/fixedobstacle.hh
worldfile.o: ../include/stage_types.hh
worldfile.o: /home/ahoward/player-1.2-alpha/include/player.h
worldfile.o: /home/ahoward/player-1.2-alpha/include/messages.h
worldfile.o: /home/ahoward/player-1.2-alpha/include/playercommon.h
worldfile.o: /home/ahoward/player-1.2-alpha/include/defaults.h
worldfile.o: ../include/worldfile.hh
