##############################################################################
#
# stage/src/Makefile - compiles all stage targets
# Author: Richard Vaughan, Andrew Howard
#
# $Id: Makefile,v 1.112 2002-07-23 16:07:56 rtv Exp $
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

# the player and stage headers everyone needs to have
INCLUDES = -I../include -I$(PLAYER_INC) 

# compile flags for devices
DEVICEFLAGS = $(INCLUDES) $(OPTIONS_COMP)

# compile flags for server 
SERVERFLAGS = $(INCLUDES) $(OPTIONS_COMP) $(ZLIB_FLAGS) \
	-DVERSION=\"$(VERSION)\" -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\"

# Link flags - optional libs linked if they were defined in Makefile.common
LINKFLAGS = -lm -lstdc++ -L$(PLAYER_DIR)/lib -lplayerqueue -lpthread\
             $(ZLIB_LINK) $(PROFILE_LINK) $(OLDLIBS) $(OPTIONS_LINK)

OBS = main.o world.o stageio.o server.o client.o worldfile.o \
	colors.o raytrace.o matrix.o image.o rtp.o entity.o entityfactory.o \
	playerdevice.o fixedobstacle.o positiondevice.o \
	omnipositiondevice.o laserdevice.o sonardevice.o \
	miscdevice.o broadcastdevice.o ptzdevice.o boxobstacle.o \
	laserbeacon.o visiondevice.o visionbeacon.o \
	puck.o gpsdevice.o gripperdevice.o motedevice.o bpsdevice.o \
	laserbeacondevice.o truthdevice.o descartesdevice.o  \
	irdevice.o idarturretdevice.o

all: stage

# STAGE ####################################################################
stage: $(OBS) rtk
	${LINKER} $(OBS) $(LINKFLAGS) -o $@

# RTK #######################################################################

rtk: $(shell ls -d ../$(RTK_DIR)/*.[ch])
	cd ../$(RTK_DIR) && ${MAKE}

# SRC TO OBJECT RULES #######################################################

# devices have the word 'device' on the end and require fewer config options
# than the simulation engine objects - makes for a tidy compile line
%device.o : %device.cc
	${CC} ${CCFLAGS} ${DEVICEFLAGS} -c $< -o $@

# anything else is considered a server file and gets all the options
%.o : %.cc
	${CC} ${CCFLAGS} ${SERVERFLAGS} -c $< -o $@


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
