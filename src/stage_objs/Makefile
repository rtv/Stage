
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
	ptzdevice.o boxobstacle.o \
	laserbeacon.o laserbeacondevice.o visiondevice.o visionbeacon.o \
	broadcastdevice.o puck.o gpsdevice.o gripperdevice.o main.o \
	matrix.o raytrace.o

# choose one of these that suits your build (debug/profile/production)

# production
CCFLAGS = -O3
# debug
#CCFLAGS = -g3 
# debug + profile
#CCFLAGS = -pg -g3

# Compile flags 
#
CCFLAGS += -Wall -DREENTRANT -I../../include -I$(PLAYER_INC) $(ZLIB_FLAGS) 
CCFLAGS += -DVERSION=\"$(VERSION)\" -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\"

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
	sed s/"^\.\.\/"// < Makefile > Makefile.bak && mv Makefile.bak Makefile

clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak

%.o : ../%.cc
	${CC} ${CCFLAGS} -c $< -o $@

# Dependancies (make dep)

boxobstacle.o: ../../include/world.hh ../../include/image.hh
boxobstacle.o: ../../include/entity.hh ../../include/stage_types.hh
boxobstacle.o: ../../include/truthserver.hh ../../include/colors.hh
boxobstacle.o: ../../include/guiexport.hh ../../include/matrix.hh
boxobstacle.o: ../../include/boxobstacle.hh
broadcastdevice.o: ../../include/world.hh ../../include/image.hh
broadcastdevice.o: ../../include/entity.hh ../../include/stage_types.hh
broadcastdevice.o: ../../include/truthserver.hh ../../include/colors.hh
broadcastdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
broadcastdevice.o: ../../include/broadcastdevice.hh
broadcastdevice.o: ../../include/playerdevice.hh
entity.o: ../../include/entity.hh ../../include/stage_types.hh
entity.o: ../../include/truthserver.hh ../../include/colors.hh
entity.o: ../../include/guiexport.hh ../../include/world.hh
entity.o: ../../include/image.hh ../../include/matrix.hh
entityfactory.o: ../../include/entityfactory.hh
entityfactory.o: ../../include/boxobstacle.hh ../../include/entity.hh
entityfactory.o: ../../include/stage_types.hh ../../include/truthserver.hh
entityfactory.o: ../../include/colors.hh ../../include/guiexport.hh
entityfactory.o: ../../include/laserbeacon.hh ../../include/puck.hh
entityfactory.o: ../../include/visionbeacon.hh
entityfactory.o: ../../include/sonardevice.hh
entityfactory.o: ../../include/playerdevice.hh
entityfactory.o: ../../include/positiondevice.hh
entityfactory.o: ../../include/laserdevice.hh ../../include/miscdevice.hh
entityfactory.o: ../../include/ptzdevice.hh ../../include/visiondevice.hh
entityfactory.o: ../../include/laserbeacondevice.hh
entityfactory.o: ../../include/broadcastdevice.hh
entityfactory.o: ../../include/gripperdevice.hh ../../include/gpsdevice.hh
entityfactory.o: ../../include/world.hh ../../include/image.hh
entityfactory.o: ../../include/matrix.hh
envserver.o: ../../include/world.hh ../../include/image.hh
envserver.o: ../../include/entity.hh ../../include/stage_types.hh
envserver.o: ../../include/truthserver.hh ../../include/colors.hh
envserver.o: ../../include/guiexport.hh ../../include/matrix.hh
gpsdevice.o: ../../include/world.hh ../../include/image.hh
gpsdevice.o: ../../include/entity.hh ../../include/stage_types.hh
gpsdevice.o: ../../include/truthserver.hh ../../include/colors.hh
gpsdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
gpsdevice.o: ../../include/gpsdevice.hh
gripperdevice.o: ../../include/world.hh ../../include/image.hh
gripperdevice.o: ../../include/entity.hh ../../include/stage_types.hh
gripperdevice.o: ../../include/truthserver.hh ../../include/colors.hh
gripperdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
gripperdevice.o: ../../include/gripperdevice.hh ../../include/raytrace.hh
image.o: ../../include/image.hh
laserbeacon.o: ../../include/world.hh ../../include/image.hh
laserbeacon.o: ../../include/entity.hh ../../include/stage_types.hh
laserbeacon.o: ../../include/truthserver.hh ../../include/colors.hh
laserbeacon.o: ../../include/guiexport.hh ../../include/matrix.hh
laserbeacon.o: ../../include/laserbeacon.hh ../../include/laserdevice.hh
laserbeacon.o: ../../include/playerdevice.hh
laserbeacondevice.o: ../../include/world.hh ../../include/image.hh
laserbeacondevice.o: ../../include/entity.hh ../../include/stage_types.hh
laserbeacondevice.o: ../../include/truthserver.hh ../../include/colors.hh
laserbeacondevice.o: ../../include/guiexport.hh ../../include/matrix.hh
laserbeacondevice.o: ../../include/laserdevice.hh
laserbeacondevice.o: ../../include/playerdevice.hh
laserbeacondevice.o: ../../include/laserbeacon.hh
laserbeacondevice.o: ../../include/laserbeacondevice.hh
laserdevice.o: ../../include/world.hh ../../include/image.hh
laserdevice.o: ../../include/entity.hh ../../include/stage_types.hh
laserdevice.o: ../../include/truthserver.hh ../../include/colors.hh
laserdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
laserdevice.o: ../../include/laserdevice.hh ../../include/playerdevice.hh
laserdevice.o: ../../include/laserbeacon.hh ../../include/raytrace.hh
main.o: ../../include/world.hh ../../include/image.hh
main.o: ../../include/entity.hh ../../include/stage_types.hh
main.o: ../../include/truthserver.hh ../../include/colors.hh
main.o: ../../include/guiexport.hh ../../include/matrix.hh
manager.o: ../../include/truthserver.hh ../../include/stage_types.hh
matrix.o: ../../include/matrix.hh ../../include/image.hh
matrix.o: ../../include/entity.hh ../../include/stage_types.hh
matrix.o: ../../include/truthserver.hh ../../include/colors.hh
matrix.o: ../../include/guiexport.hh
miscdevice.o: ../../include/world.hh ../../include/image.hh
miscdevice.o: ../../include/entity.hh ../../include/stage_types.hh
miscdevice.o: ../../include/truthserver.hh ../../include/colors.hh
miscdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
miscdevice.o: ../../include/miscdevice.hh ../../include/playerdevice.hh
occupancydevice.o: ../../include/world.hh ../../include/image.hh
occupancydevice.o: ../../include/entity.hh ../../include/stage_types.hh
occupancydevice.o: ../../include/truthserver.hh ../../include/colors.hh
occupancydevice.o: ../../include/guiexport.hh ../../include/matrix.hh
occupancydevice.o: ../../include/occupancydevice.hh
playerdevice.o: ../../include/world.hh ../../include/image.hh
playerdevice.o: ../../include/entity.hh ../../include/stage_types.hh
playerdevice.o: ../../include/truthserver.hh ../../include/colors.hh
playerdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
playerdevice.o: ../../include/playerdevice.hh
positiondevice.o: ../../include/world.hh ../../include/image.hh
positiondevice.o: ../../include/entity.hh ../../include/stage_types.hh
positiondevice.o: ../../include/truthserver.hh ../../include/colors.hh
positiondevice.o: ../../include/guiexport.hh ../../include/matrix.hh
positiondevice.o: ../../include/positiondevice.hh
positiondevice.o: ../../include/raytrace.hh
ptzdevice.o: ../../include/world.hh ../../include/image.hh
ptzdevice.o: ../../include/entity.hh ../../include/stage_types.hh
ptzdevice.o: ../../include/truthserver.hh ../../include/colors.hh
ptzdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
ptzdevice.o: ../../include/ptzdevice.hh ../../include/playerdevice.hh
puck.o: ../../include/world.hh ../../include/image.hh
puck.o: ../../include/entity.hh ../../include/stage_types.hh
puck.o: ../../include/truthserver.hh ../../include/colors.hh
puck.o: ../../include/guiexport.hh ../../include/matrix.hh
puck.o: ../../include/puck.hh ../../include/raytrace.hh
raytrace.o: ../../include/raytrace.hh ../../include/matrix.hh
raytrace.o: ../../include/image.hh ../../include/entity.hh
raytrace.o: ../../include/stage_types.hh ../../include/truthserver.hh
raytrace.o: ../../include/colors.hh ../../include/guiexport.hh
render.o: ../../include/stage_types.hh ../../include/truthserver.hh
render.o: ../../include/xs.hh
rtk_main.o: ../../include/world.hh ../../include/image.hh
rtk_main.o: ../../include/entity.hh ../../include/stage_types.hh
rtk_main.o: ../../include/truthserver.hh ../../include/colors.hh
rtk_main.o: ../../include/guiexport.hh ../../include/matrix.hh
sonardevice.o: ../../include/world.hh ../../include/image.hh
sonardevice.o: ../../include/entity.hh ../../include/stage_types.hh
sonardevice.o: ../../include/truthserver.hh ../../include/colors.hh
sonardevice.o: ../../include/guiexport.hh ../../include/matrix.hh
sonardevice.o: ../../include/sonardevice.hh ../../include/playerdevice.hh
sonardevice.o: ../../include/raytrace.hh
truthdevice.o: ../../include/world.hh ../../include/image.hh
truthdevice.o: ../../include/entity.hh ../../include/stage_types.hh
truthdevice.o: ../../include/truthserver.hh ../../include/colors.hh
truthdevice.o: ../../include/guiexport.hh ../../include/matrix.hh
truthdevice.o: ../../include/truthdevice.hh
truthlog.o: ../../include/truthserver.hh ../../include/stage_types.hh
truthserver.o: ../../include/world.hh ../../include/image.hh
truthserver.o: ../../include/entity.hh ../../include/stage_types.hh
truthserver.o: ../../include/truthserver.hh ../../include/colors.hh
truthserver.o: ../../include/guiexport.hh ../../include/matrix.hh
visionbeacon.o: ../../include/world.hh ../../include/image.hh
visionbeacon.o: ../../include/entity.hh ../../include/stage_types.hh
visionbeacon.o: ../../include/truthserver.hh ../../include/colors.hh
visionbeacon.o: ../../include/guiexport.hh ../../include/matrix.hh
visionbeacon.o: ../../include/visionbeacon.hh
visiondevice.o: ../../include/world.hh ../../include/image.hh
visiondevice.o: ../../include/entity.hh ../../include/stage_types.hh
visiondevice.o: ../../include/truthserver.hh ../../include/colors.hh
visiondevice.o: ../../include/guiexport.hh ../../include/matrix.hh
visiondevice.o: ../../include/visiondevice.hh ../../include/ptzdevice.hh
visiondevice.o: ../../include/playerdevice.hh ../../include/raytrace.hh
world.o: ../../include/world.hh ../../include/image.hh
world.o: ../../include/entity.hh ../../include/stage_types.hh
world.o: ../../include/truthserver.hh ../../include/colors.hh
world.o: ../../include/guiexport.hh ../../include/matrix.hh
world.o: ../../include/playerdevice.hh
world_load.o: ../../include/stage_types.hh ../../include/world.hh
world_load.o: ../../include/image.hh ../../include/entity.hh
world_load.o: ../../include/truthserver.hh ../../include/colors.hh
world_load.o: ../../include/guiexport.hh ../../include/matrix.hh
world_load.o: ../../include/entityfactory.hh
xs.o: ../../include/stage_types.hh ../../include/truthserver.hh
xs.o: ../../include/xs.hh
