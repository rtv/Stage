
##############################################################################
# include the main configurations from the top-level directory
#

include ../../Makefile.common

#############################################################################
# These are the key paths needed for the build

RTK_DIR = ../../rtk
# rtkstage needs RTK installed and these paths set correctly
# RTK headers
RTK_INC = $(RTK_DIR)/include
# RTK libraries
RTK_LIB = $(RTK_DIR)/lib

##############################################################################
# Build section

# Objects common to all targets
#
OBJS =  world.o world_load.o truthserver.o envserver.o image.o \
	entity.o entityfactory.o playerdevice.o \
	positiondevice.o laserdevice.o sonardevice.o miscdevice.o \
	ptzdevice.o boxobstacle.o \
	laserbeacon.o laserbeacondevice.o visiondevice.o visionbeacon.o \
	broadcastdevice.o puck.o gpsdevice.o gripperdevice.o rtk_main.o \
	matrix.o raytrace.o

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
CCFLAGS += -DINCLUDE_RTK -I$(RTK_INC) `gtk-config --cflags`
CCFLAGS += -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\"

# production/debug link
LINKFLAGS = -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
# profile link
#LINKFLAGS = -pg -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
LINKFLAGS += -L$(RTK_LIB) -lrtk `gtk-config --libs gtk gthread`

# Standard tools
#
CC := g++
LINKER = g++
MAKEDEP = makedepend

.SUFFIXES: .cc .o

##############################################################################
# Targets

all: rtkstage 

# Target stage with RTK GUI
#
rtkstage : $(OBJS) rtk
	$(LINKER) $(OBJS) $(LINKFLAGS) -o $@

rtk:
	cd $(RTK_DIR) && make

clean:
	rm -f *.o *~ core rtkstage

dep:
	$(MAKEDEP) -Y -- $(CCFLAGS) -- -s "# Dependancies (make dep)" ../*.cc 2>/dev/null
	$(RM) -f Makefile.bak

clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak

%.o : ../%.cc
	$(CC) $(CCFLAGS) -c $< -o $@

# Dependancies (make dep)


