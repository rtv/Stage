
##############################################################################
# include the main configurations from the top-level directory
#

include ../../Makefile.common

##############################################################################
# Build section

# Stage object files
#
#OBJS =  world.o worldfile.o truthserver.o envserver.o image.o \
	#entity.o entityfactory.o playerdevice.o fixedobstacle.o \
	#positiondevice.o omnipositiondevice.o laserdevice.o sonardevice.o \
	#miscdevice.o broadcastdevice.o \
	#ptzdevice.o boxobstacle.o \
	#laserbeacon.o laserbeacondevice.o visiondevice.o visionbeacon.o \
	#puck.o gpsdevice.o gripperdevice.o main.o \
	#matrix.o raytrace.o 
#irdevice.o descartesdevice.o

OBS = ${SRCS:.cc=.o}

# choose one of these that suits your build (debug/profile/production)

# production
#CCFLAGS = -O3
# debug
CCFLAGS = -g3 
# debug + profile
#CCFLAGS = -pg -g3

# Compile flags 
#
CCFLAGS += -Wall -DREENTRANT $(ZLIB_FLAGS) 
CCFLAGS += -I../../include -I$(PLAYER_INC) $(HRL_HEADERS) 
CCFLAGS += -DVERSION=\"$(VERSION)\" -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\"

# production/debug link
#LINKFLAGS = -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
# profile link
LINKFLAGS = -pg -lm -lstdc++ -lpthread  $(ZLIB_LIBS)

.SUFFIXES: .cc .o

##############################################################################
# Targets

all: stage

# Target stage with no GUI
#
#stage : $(OBJS)
#	$(LINKER) $(LINKFLAGS) -o $@ $(OBJS)  

stage : $(OBS)
	$(LINKER) $(LINKFLAGS) -o $@ $(OBS) 

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
