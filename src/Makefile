##############################################################################
#
# stage/src/Makefile - compiles all stage targets
# Author: Richard Vaughan, Andrew Howard
#
# $Id: Makefile,v 1.107.4.2 2002-07-15 20:39:15 rtv Exp $
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
	puck.o gpsdevice.o gripperdevice.o bpsdevice.o \
	laserbeacondevice.o truthdevice.o
#motedevice.o 

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
