##############################################################################
#
# stage/src/Makefile - compiles all stage targets
# Author: Richard Vaughan, Andrew Howard
#
# $Id: Makefile,v 1.104 2002-06-11 03:31:18 inspectorg Exp $
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

OBS = main.o world.o stageio.o server.o client.o worldfile.o \
	colors.o raytrace.o matrix.o image.o rtp.o entity.o entityfactory.o \
	playerdevice.o fixedobstacle.o 	positiondevice.o \
	omnipositiondevice.o laserdevice.o sonardevice.o \
	miscdevice.o broadcastdevice.o ptzdevice.o boxobstacle.o \
	laserbeacon.o visiondevice.o visionbeacon.o \
	puck.o gpsdevice.o gripperdevice.o motedevice.o bpsdevice.o \
	laserbeacondevice.o truthdevice.o


# TARGETS  #########################################################
all: stage #tools 

# STAGE ####################################################################
#
# in order to cut down on unnecessary recompilation, make Stage depend
# directly on the *.c and *.h files that comprise rtk - BPG
RTKFILES = $(shell ls -d ../$(RTK_DIR)/*.[ch])
stage: $(RTKFILES) $(OBS)
	cd ../$(RTK_DIR) && ${MAKE}
	$(LINKER) $(OBS) $(STAGELINK) -o $@

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

# makedepend.  use it; it's your friend.  
dep:
	$(MAKEDEP) -a -o.o -Y $(INCLUDES) -s "# Dependancies (make dep)" *.cc 2> /dev/null
	rm -f Makefile.bak

clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak

# Dependancies (make dep)

boxobstacle.o: ../include/world.hh ../include/image.hh ../include/entity.hh
boxobstacle.o: ../include/stage_types.hh ../include/colors.hh
boxobstacle.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
boxobstacle.o: ../include/boxobstacle.hh
bpsdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
bpsdevice.o: ../include/stage_types.hh ../include/colors.hh ../include/rtp.h
bpsdevice.o: ../include/matrix.hh ../include/worldfile.hh
bpsdevice.o: ../include/bpsdevice.hh ../include/playerdevice.hh
broadcastdevice.o: ../include/world.hh ../include/image.hh
broadcastdevice.o: ../include/entity.hh ../include/stage_types.hh
broadcastdevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
broadcastdevice.o: ../include/worldfile.hh ../include/broadcastdevice.hh
broadcastdevice.o: ../include/playerdevice.hh
client.o: ../include/server.hh ../include/stage_types.hh
client.o: ../include/worldfile.hh ../include/world.hh ../include/image.hh
client.o: ../include/entity.hh ../include/colors.hh ../include/rtp.h
client.o: ../include/matrix.hh
colors.o: ../include/stage_types.hh ../include/colors.hh
entity.o: ../include/entity.hh ../include/stage_types.hh ../include/colors.hh
entity.o: ../include/rtp.h ../include/raytrace.hh ../include/matrix.hh
entity.o: ../include/image.hh ../include/world.hh ../include/worldfile.hh
entityfactory.o: ../include/entityfactory.hh ../include/fixedobstacle.hh
entityfactory.o: ../include/entity.hh ../include/stage_types.hh
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
envserver.o: ../include/world.hh ../include/image.hh ../include/entity.hh
envserver.o: ../include/stage_types.hh ../include/colors.hh ../include/rtp.h
envserver.o: ../include/matrix.hh ../include/worldfile.hh
envserver.o: ../include/truthserver.hh
fixedobstacle.o: ../include/image.hh ../include/world.hh ../include/entity.hh
fixedobstacle.o: ../include/stage_types.hh ../include/colors.hh
fixedobstacle.o: ../include/rtp.h ../include/matrix.hh
fixedobstacle.o: ../include/worldfile.hh ../include/fixedobstacle.hh
gpsdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
gpsdevice.o: ../include/stage_types.hh ../include/colors.hh ../include/rtp.h
gpsdevice.o: ../include/matrix.hh ../include/worldfile.hh
gpsdevice.o: ../include/gpsdevice.hh
gripperdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
gripperdevice.o: ../include/stage_types.hh ../include/colors.hh
gripperdevice.o: ../include/rtp.h ../include/matrix.hh
gripperdevice.o: ../include/worldfile.hh ../include/gripperdevice.hh
gripperdevice.o: ../include/raytrace.hh
image.o: ../include/stage_types.hh ../include/image.hh
laserbeacon.o: ../include/world.hh ../include/image.hh ../include/entity.hh
laserbeacon.o: ../include/stage_types.hh ../include/colors.hh
laserbeacon.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
laserbeacon.o: ../include/laserbeacon.hh ../include/laserdevice.hh
laserbeacon.o: ../include/playerdevice.hh
laserbeacondevice.o: ../include/world.hh ../include/image.hh
laserbeacondevice.o: ../include/entity.hh ../include/stage_types.hh
laserbeacondevice.o: ../include/colors.hh ../include/rtp.h
laserbeacondevice.o: ../include/matrix.hh ../include/worldfile.hh
laserbeacondevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacon.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh ../include/guiexport.hh
laserdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
laserdevice.o: ../include/stage_types.hh ../include/colors.hh
laserdevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
laserdevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserdevice.o: ../include/laserbeacon.hh ../include/raytrace.hh
main.o: ../include/server.hh ../include/stage_types.hh
main.o: ../include/worldfile.hh ../include/world.hh ../include/image.hh
main.o: ../include/entity.hh ../include/colors.hh ../include/rtp.h
main.o: ../include/matrix.hh
manager.o: ../include/truthserver.hh ../include/stage_types.hh
matrix.o: ../include/matrix.hh ../include/image.hh ../include/entity.hh
matrix.o: ../include/stage_types.hh ../include/colors.hh ../include/rtp.h
miscdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
miscdevice.o: ../include/stage_types.hh ../include/colors.hh ../include/rtp.h
miscdevice.o: ../include/matrix.hh ../include/worldfile.hh
miscdevice.o: ../include/miscdevice.hh ../include/playerdevice.hh
motedevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
motedevice.o: ../include/stage_types.hh ../include/colors.hh ../include/rtp.h
motedevice.o: ../include/matrix.hh ../include/worldfile.hh
motedevice.o: ../include/motedevice.hh
omnipositiondevice.o: ../include/world.hh ../include/image.hh
omnipositiondevice.o: ../include/entity.hh ../include/stage_types.hh
omnipositiondevice.o: ../include/colors.hh ../include/rtp.h
omnipositiondevice.o: ../include/matrix.hh ../include/worldfile.hh
omnipositiondevice.o: ../include/raytrace.hh ../include/omnipositiondevice.hh
playerdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
playerdevice.o: ../include/stage_types.hh ../include/colors.hh
playerdevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
playerdevice.o: ../include/playerdevice.hh
positiondevice.o: ../include/world.hh ../include/image.hh
positiondevice.o: ../include/entity.hh ../include/stage_types.hh
positiondevice.o: ../include/colors.hh ../include/rtp.h ../include/matrix.hh
positiondevice.o: ../include/worldfile.hh ../include/positiondevice.hh
ptzdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
ptzdevice.o: ../include/stage_types.hh ../include/colors.hh ../include/rtp.h
ptzdevice.o: ../include/matrix.hh ../include/worldfile.hh
ptzdevice.o: ../include/ptzdevice.hh ../include/playerdevice.hh
puck.o: ../include/world.hh ../include/image.hh ../include/entity.hh
puck.o: ../include/stage_types.hh ../include/colors.hh ../include/rtp.h
puck.o: ../include/matrix.hh ../include/worldfile.hh ../include/puck.hh
puck.o: ../include/raytrace.hh
raytrace.o: ../include/raytrace.hh ../include/matrix.hh ../include/image.hh
raytrace.o: ../include/entity.hh ../include/stage_types.hh
raytrace.o: ../include/colors.hh ../include/rtp.h ../include/world.hh
raytrace.o: ../include/worldfile.hh
rtp.o: ../include/rtp.h
server.o: ../include/server.hh ../include/stage_types.hh
server.o: ../include/worldfile.hh ../include/world.hh ../include/image.hh
server.o: ../include/entity.hh ../include/colors.hh ../include/rtp.h
server.o: ../include/matrix.hh ../include/fixedobstacle.hh
sonardevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
sonardevice.o: ../include/stage_types.hh ../include/colors.hh
sonardevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
sonardevice.o: ../include/sonardevice.hh ../include/playerdevice.hh
sonardevice.o: ../include/raytrace.hh
stageio.o: ../include/server.hh ../include/stage_types.hh
stageio.o: ../include/worldfile.hh ../include/world.hh ../include/image.hh
stageio.o: ../include/entity.hh ../include/colors.hh ../include/rtp.h
stageio.o: ../include/matrix.hh ../include/fixedobstacle.hh
truthdevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
truthdevice.o: ../include/stage_types.hh ../include/colors.hh
truthdevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
truthdevice.o: ../include/truthdevice.hh
truthlog.o: ../include/truthserver.hh ../include/stage_types.hh
visionbeacon.o: ../include/world.hh ../include/image.hh ../include/entity.hh
visionbeacon.o: ../include/stage_types.hh ../include/colors.hh
visionbeacon.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
visionbeacon.o: ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh ../include/image.hh ../include/entity.hh
visiondevice.o: ../include/stage_types.hh ../include/colors.hh
visiondevice.o: ../include/rtp.h ../include/matrix.hh ../include/worldfile.hh
visiondevice.o: ../include/visiondevice.hh ../include/ptzdevice.hh
visiondevice.o: ../include/playerdevice.hh ../include/raytrace.hh
world.o: ../include/world.hh ../include/image.hh ../include/entity.hh
world.o: ../include/stage_types.hh ../include/colors.hh ../include/rtp.h
world.o: ../include/matrix.hh ../include/worldfile.hh
world.o: ../include/fixedobstacle.hh
worldfile.o: ../include/stage_types.hh ../include/colors.hh
worldfile.o: ../include/worldfile.hh
