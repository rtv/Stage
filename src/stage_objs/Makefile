
##############################################################################
# include the main configurations from the top-level directory
#

include ../../Makefile.common

##############################################################################
# Build section

# Objects common to all targets
#
OBJS =  world.o world_load.o worldfile.o \
	truthserver.o envserver.o image.o \
	entity.o entityfactory.o playerdevice.o \
	positiondevice.o omnipositiondevice.o laserdevice.o sonardevice.o miscdevice.o \
	ptzdevice.o boxobstacle.o \
	laserbeacon.o laserbeacondevice.o visiondevice.o visionbeacon.o \
	puck.o broadcastdevice.o gpsdevice.o gripperdevice.o main.o \
	matrix.o raytrace.o irdevice.o

# choose one of these that suits your build (debug/profile/production)

# production
#CCFLAGS = -O3
# debug
CCFLAGS = -g3 
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
