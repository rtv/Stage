##############################################################################
#
# stage/src/Makefile - compiles all stage targets
# Author: Richard Vaughan, Andrew Howard
#
# $Id: Makefile,v 1.99 2002-06-09 21:16:14 gerkey Exp $
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

# makedepend.  use it; it's your friend.  
dep:
	$(MAKEDEP) -a -o.o -Y $(INCLUDES) -s "# Dependancies (make dep)" *.cc 2> /dev/null
	rm -f Makefile.bak

clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak

# Dependancies (make dep)
