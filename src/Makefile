

##############################################################################
# These are the key paths needs for the build and install

PLAYER_INC = $(PLAYER_DIR)/include
PLAYER_LIB = $(PLAYER_DIR)/lib
RTK_INC = $(RTK_DIR)/include
RTK_LIB = $(RTK_DIR)/lib
INSTALL_BIN = $(INSTALL_DIR)/bin
INSTALL_ETC = $(INSTALL_DIR)/etc


##############################################################################
# Build section

# Objects common to all targets
#
OBJS = world.o image.o \
		object.o objectfactory.o tokenize.o \
		playerrobot.o playerdevice.o \
		pioneermobiledevice.o laserdevice.o \
		visiondevice.o ptzdevice.o sonardevice.o \
		laserbeacondevice.o broadcastdevice.o miscdevice.o \
		boxobstacle.o laserbeacon.o visionbeacon.o \
		usc_pioneer.o

# Flags common to all targets
#
CCFLAGS = -g3 -Wall -I../include -I$(PLAYER_INC)
LINKFLAGS = -lm -lstdc++ -lpthread -L$(PLAYER_LIB) -lplayerclient

# Standard tools
#
CC := g++
LINKER = gcc
MAKEDEP = makedepend

.SUFFIXES: .cc .o

# Target stage with no GUI
#
stage : OBJS += main.o
stage : $(OBJS) main.o 
	${LINKER} $(OBJS) $(LINKFLAGS) -o stage

# Target stage with RTK GUI
#
rtk_stage : OBJS += rtk_main.o
rtk_stage : CCFLAGS += -DINCLUDE_RTK -I$(RTK_INC) `gtk-config --cflags`
rtk_stage : LINKFLAGS += -L$(RTK_LIB) -lrtk `gtk-config --libs gtk gthread`
rtk_stage : $(OBJS) rtk_main.o 
	${LINKER} $(OBJS) $(LINKFLAGS) -o rtk_stage

clean:
	rm -f *.o *~ core

dep:
	$(MAKEDEP) -Y -I../include -I$(PLAYER_INC) -s "# Dependancies (make dep)" *.cc 2>/dev/null

%.o : %.cc
	${CC} ${CCFLAGS} -c $< -o $@


##############################################################################
# Install section
# Installs the executable in the Player bin directory
# You may need to su root to do this

INSTALL = install
MKDIR = mkdir
CP = cp

install:
	$(MKDIR) -p $(INSTALL_BIN)
	$(INSTALL) -m 755 stage $(INSTALL_BIN)
	$(INSTALL) -m 755 rtk_stage $(INSTALL_BIN)
	$(MKDIR) -p $(INSTALL_ETC)
	$(INSTALL) -m 755 rtk_stage.cfg $(INSTALL_ETC)


##############################################################################
# Dependancies (make dep)

boxobstacle.o: ../include/world.hh ../include/playerrobot.hh
boxobstacle.o: ../include/object.hh ../include/stage_types.hh
boxobstacle.o: ../include/image.h ../include/tokenize.hh
boxobstacle.o: ../include/boxobstacle.hh
broadcastdevice.o: ../include/world.hh ../include/playerrobot.hh
broadcastdevice.o: ../include/object.hh ../include/stage_types.hh
broadcastdevice.o: ../include/image.h ../include/broadcastdevice.hh
broadcastdevice.o: ../include/playerdevice.hh
image.o: ../include/image.h
laserbeacon.o: ../include/world.hh ../include/playerrobot.hh
laserbeacon.o: ../include/object.hh ../include/stage_types.hh
laserbeacon.o: ../include/image.h ../include/tokenize.hh
laserbeacon.o: ../include/laserbeacon.hh
laserbeacondevice.o: ../include/world.hh ../include/playerrobot.hh
laserbeacondevice.o: ../include/object.hh ../include/stage_types.hh
laserbeacondevice.o: ../include/image.h ../include/laserdevice.hh
laserbeacondevice.o: ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh
laserdevice.o: ../include/world.hh ../include/playerrobot.hh
laserdevice.o: ../include/object.hh ../include/stage_types.hh
laserdevice.o: ../include/image.h ../include/laserdevice.hh
laserdevice.o: ../include/playerdevice.hh
main.o: ../VERSION ../include/world.hh ../include/playerrobot.hh
main.o: ../include/object.hh ../include/stage_types.hh ../include/image.h
miscdevice.o: ../include/world.hh ../include/playerrobot.hh
miscdevice.o: ../include/object.hh ../include/stage_types.hh
miscdevice.o: ../include/image.h ../include/miscdevice.hh
miscdevice.o: ../include/playerdevice.hh
object.o: ../include/object.hh ../include/stage_types.hh
objectfactory.o: ../include/objectfactory.hh ../include/boxobstacle.hh
objectfactory.o: ../include/object.hh ../include/stage_types.hh
objectfactory.o: ../include/laserbeacon.hh ../include/visionbeacon.hh
objectfactory.o: ../include/usc_pioneer.hh
pioneermobiledevice.o: ../include/world.hh ../include/playerrobot.hh
pioneermobiledevice.o: ../include/object.hh ../include/stage_types.hh
pioneermobiledevice.o: ../include/image.h ../include/pioneermobiledevice.hh
pioneermobiledevice.o: ../include/playerdevice.hh
playerdevice.o: ../include/world.hh ../include/playerrobot.hh
playerdevice.o: ../include/object.hh ../include/stage_types.hh
playerdevice.o: ../include/image.h ../include/playerdevice.hh
playerrobot.o: ../include/world.hh ../include/playerrobot.hh
playerrobot.o: ../include/object.hh ../include/stage_types.hh
playerrobot.o: ../include/image.h
posreader.o: ../include/ports.h
ptzdevice.o: ../include/world.hh ../include/playerrobot.hh
ptzdevice.o: ../include/object.hh ../include/stage_types.hh
ptzdevice.o: ../include/image.h ../include/ptzdevice.hh
ptzdevice.o: ../include/playerdevice.hh
rtk_main.o: ../VERSION ../include/world.hh ../include/playerrobot.hh
rtk_main.o: ../include/object.hh ../include/stage_types.hh ../include/image.h
server.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
server.o: ../include/stage_types.hh ../include/image.h ../include/ports.h
sonardevice.o: ../include/world.hh ../include/playerrobot.hh
sonardevice.o: ../include/object.hh ../include/stage_types.hh
sonardevice.o: ../include/image.h ../include/sonardevice.hh
sonardevice.o: ../include/playerdevice.hh
tokenize.o: ../include/tokenize.hh
usc_pioneer.o: ../include/world.hh ../include/playerrobot.hh
usc_pioneer.o: ../include/object.hh ../include/stage_types.hh
usc_pioneer.o: ../include/image.h ../include/tokenize.hh
usc_pioneer.o: ../include/usc_pioneer.hh ../include/pioneermobiledevice.hh
usc_pioneer.o: ../include/playerdevice.hh ../include/miscdevice.hh
usc_pioneer.o: ../include/sonardevice.hh ../include/laserdevice.hh
visionbeacon.o: ../include/world.hh ../include/playerrobot.hh
visionbeacon.o: ../include/object.hh ../include/stage_types.hh
visionbeacon.o: ../include/image.h ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh ../include/playerrobot.hh
visiondevice.o: ../include/object.hh ../include/stage_types.hh
visiondevice.o: ../include/image.h ../include/visiondevice.hh
visiondevice.o: ../include/playerdevice.hh ../include/ptzdevice.hh
win.o: ../include/win.h ../include/world.hh ../include/playerrobot.hh
win.o: ../include/object.hh ../include/stage_types.hh ../include/image.h
world.o: ../include/tokenize.hh ../include/world.hh ../include/playerrobot.hh
world.o: ../include/object.hh ../include/stage_types.hh ../include/image.h
world.o: ../include/objectfactory.hh
