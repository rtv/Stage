

##############################################################################
# These are the key paths needs for the build and install

PLAYER_DIR = /usr/local/player
PLAYER_INC = $(PLAYER_DIR)/include
PLAYER_LIB = $(PLAYER_DIR)/lib
RTK_DIR = /usr/local/rtk
RTK_INC = $(RTK_DIR)/include
RTK_LIB = $(RTK_DIR)/lib
INSTALL_BIN = $(INSTALL_DIR)/bin
INSTALL_ETC = $(INSTALL_DIR)/etc


##############################################################################
# Build section

# Objects common to all targets
#
OBJS = world.o image.o \
		object.o objectfactory.o \
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
boxobstacle.o: ../include/object.hh ../include/image.h
boxobstacle.o: ../include/boxobstacle.hh
broadcastdevice.o: /usr/local/player/include/stage.h
broadcastdevice.o: /usr/local/player/include/messages.h
broadcastdevice.o: /usr/local/player/include/playercommon.h
broadcastdevice.o: ../include/world.hh ../include/playerrobot.hh
broadcastdevice.o: ../include/object.hh ../include/image.h
broadcastdevice.o: ../include/broadcastdevice.hh ../include/playerdevice.hh
image.o: ../include/image.h
laserbeacon.o: ../include/world.hh ../include/playerrobot.hh
laserbeacon.o: ../include/object.hh ../include/image.h
laserbeacon.o: ../include/laserbeacon.hh
laserbeacondevice.o: /usr/local/player/include/stage.h
laserbeacondevice.o: /usr/local/player/include/messages.h
laserbeacondevice.o: /usr/local/player/include/playercommon.h
laserbeacondevice.o: ../include/world.hh ../include/playerrobot.hh
laserbeacondevice.o: ../include/object.hh ../include/image.h
laserbeacondevice.o: ../include/laserdevice.hh ../include/playerdevice.hh
laserbeacondevice.o: ../include/laserbeacondevice.hh
laserdevice.o: /usr/local/player/include/stage.h
laserdevice.o: /usr/local/player/include/messages.h
laserdevice.o: /usr/local/player/include/playercommon.h ../include/world.hh
laserdevice.o: ../include/playerrobot.hh ../include/object.hh
laserdevice.o: ../include/image.h ../include/laserdevice.hh
laserdevice.o: ../include/playerdevice.hh
main.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
main.o: ../include/image.h
miscdevice.o: /usr/local/player/include/stage.h
miscdevice.o: /usr/local/player/include/messages.h
miscdevice.o: /usr/local/player/include/playercommon.h ../include/world.hh
miscdevice.o: ../include/playerrobot.hh ../include/object.hh
miscdevice.o: ../include/image.h ../include/miscdevice.hh
miscdevice.o: ../include/playerdevice.hh
object.o: ../include/object.hh ../include/objectfactory.hh
objectfactory.o: ../include/objectfactory.hh ../include/boxobstacle.hh
objectfactory.o: ../include/object.hh ../include/laserbeacon.hh
objectfactory.o: ../include/visionbeacon.hh ../include/usc_pioneer.hh
pioneermobiledevice.o: ../include/world.hh ../include/playerrobot.hh
pioneermobiledevice.o: ../include/object.hh ../include/image.h
pioneermobiledevice.o: ../include/pioneermobiledevice.hh
pioneermobiledevice.o: /usr/local/player/include/stage.h
pioneermobiledevice.o: /usr/local/player/include/messages.h
pioneermobiledevice.o: /usr/local/player/include/playercommon.h
pioneermobiledevice.o: ../include/playerdevice.hh
playerdevice.o: ../include/world.hh ../include/playerrobot.hh
playerdevice.o: ../include/object.hh ../include/image.h
playerdevice.o: ../include/playerdevice.hh /usr/local/player/include/stage.h
playerdevice.o: /usr/local/player/include/messages.h
playerdevice.o: /usr/local/player/include/playercommon.h
playerrobot.o: /usr/local/player/include/stage.h
playerrobot.o: /usr/local/player/include/messages.h
playerrobot.o: /usr/local/player/include/playercommon.h ../include/world.hh
playerrobot.o: ../include/playerrobot.hh ../include/object.hh
playerrobot.o: ../include/image.h
posreader.o: ../include/ports.h
ptzdevice.o: ../include/world.hh ../include/playerrobot.hh
ptzdevice.o: ../include/object.hh ../include/image.h ../include/ptzdevice.hh
ptzdevice.o: ../include/playerdevice.hh /usr/local/player/include/stage.h
ptzdevice.o: /usr/local/player/include/messages.h
ptzdevice.o: /usr/local/player/include/playercommon.h
server.o: ../include/world.hh ../include/playerrobot.hh ../include/object.hh
server.o: ../include/image.h ../include/ports.h
sonardevice.o: ../include/world.hh ../include/playerrobot.hh
sonardevice.o: ../include/object.hh ../include/image.h
sonardevice.o: ../include/sonardevice.hh ../include/playerdevice.hh
sonardevice.o: /usr/local/player/include/stage.h
sonardevice.o: /usr/local/player/include/messages.h
sonardevice.o: /usr/local/player/include/playercommon.h
usc_pioneer.o: ../include/world.hh ../include/playerrobot.hh
usc_pioneer.o: ../include/object.hh ../include/image.h
usc_pioneer.o: ../include/usc_pioneer.hh ../include/pioneermobiledevice.hh
usc_pioneer.o: /usr/local/player/include/stage.h
usc_pioneer.o: /usr/local/player/include/messages.h
usc_pioneer.o: /usr/local/player/include/playercommon.h
usc_pioneer.o: ../include/playerdevice.hh ../include/miscdevice.hh
usc_pioneer.o: ../include/sonardevice.hh ../include/laserdevice.hh
visionbeacon.o: ../include/world.hh ../include/playerrobot.hh
visionbeacon.o: ../include/object.hh ../include/image.h
visionbeacon.o: ../include/visionbeacon.hh
visiondevice.o: ../include/world.hh ../include/playerrobot.hh
visiondevice.o: ../include/object.hh ../include/image.h
visiondevice.o: ../include/visiondevice.hh ../include/playerdevice.hh
visiondevice.o: /usr/local/player/include/stage.h
visiondevice.o: /usr/local/player/include/messages.h
visiondevice.o: /usr/local/player/include/playercommon.h
visiondevice.o: ../include/ptzdevice.hh
win.o: ../include/win.h ../include/world.hh ../include/playerrobot.hh
win.o: ../include/object.hh ../include/image.h
world.o: /usr/local/player/include/stage.h
world.o: /usr/local/player/include/messages.h
world.o: /usr/local/player/include/playercommon.h ../include/world.hh
world.o: ../include/playerrobot.hh ../include/object.hh ../include/image.h
world.o: ../include/objectfactory.hh
