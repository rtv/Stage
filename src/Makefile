##############################################################################
#
# stage/src/Makefile - compiles all stage targets
# Author: Richard Vaughan, Andrew Howard
#
# $Id: Makefile,v 1.81 2002-03-15 02:52:16 rtv Exp $
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

# Compile flags common to all versions of Stage
STAGEFLAGS = -DREENTRANT $(ZLIB_FLAGS) $(INCLUDES) $(PLAYER_PLATFORM) \
             -DVERSION=\"$(VERSION)\" -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\" \
             $(HRL_FLAGS)	

# Link flags common to all versions of Stage
# optional libraries are linked if they were defined in Makefile.common
STAGELINK =  -lm -lstdc++ -pthread \
             $(ZLIB_LINK) $(PROFILE_LINK) $(HRL_LINK) $(OLDLIBS) 

##############################################################################
# Define the sources to build from
#
SRCS = world.cc worldfile.cc truthserver.cc envserver.cc image.cc \
	entity.cc entityfactory.cc playerdevice.cc fixedobstacle.cc \
	positiondevice.cc omnipositiondevice.cc laserdevice.cc sonardevice.cc \
	miscdevice.cc broadcastdevice.cc \
	ptzdevice.cc boxobstacle.cc \
	laserbeacon.cc laserbeacondevice.cc visiondevice.cc visionbeacon.cc \
	puck.cc gpsdevice.cc gripperdevice.cc main.cc \
	matrix.cc raytrace.cc motedevice.cc

# stage and rtkstage objects are compiled from the same sources, but rtkstage
# defines a couple of macros to enable some optional code
#
# stage objects have a _stg.o suffix
# rtkstage objects have a _rtk.o suffix
STG_OBS = ${SRCS:.cc=_stg.o}
RTK_OBS = ${SRCS:.cc=_rtk.o}

# use the suffix rules below to munge files with these suffixes
#SUFFIXES = .o .cc

# TARGETS  #########################################################

typical: stage rtkstage xs manager 
all: typical trapdoor truthlog

# STAGE ####################################################################

stage: ${STG_OBS}
	$(LINKER) $(STG_OBS) $(STAGELINK) -o $@

%_stg.o : %.cc
	${CC} ${CCFLAGS} ${STAGEFLAGS} -c $< -o $@

# RTKSTAGE ##################################################################
# rtkstage links with rtk and gtk
RTKSTAGEFLAGS = ${CCFLAGS} ${STAGEFLAGS} \
                -DINCLUDE_RTK2 -I../$(RTK_INC) `gtk-config --cflags`

RTKSTAGELINK = $(STAGELINK)  \
               -L../$(RTK_LIB) -lrtk `gtk-config --libs gtk gthread`

rtkstage: rtk ${RTK_OBS} 
	$(LINKER) $(RTK_OBS) $(RTKSTAGELINK) -o $@ 

%_rtk.o : %.cc
	${CC} ${RTKSTAGEFLAGS} -c $< -o $@

rtk:
	cd ../$(RTK_DIR) && ${MAKE}

# XS ######################################################################

XSFLAGS = $(CCFLAGS) ${INCLUDES} $(HRL_FLAGS) -I/usr/X11R6/include 
XSLINK = -L/usr/X11R6/lib -lX11 -L$(PLAYER_LIB) -lplayerclient -lpthread

xs: xs.cc render.cc
	$(CC) xs.cc render.cc  $(XSFLAGS) $(XSLINK) -o xs

# TOOLS ###################################################################

manager: manager.cc 
	$(CC) -I../include -Wall -I$(PLAYER_INC) -lstdc++ manager.cc -o manager

# truthlog doesn't compile... BPG 'cos it's broken... RTV
truthlog: truthlog.cc 
	$(CC) -I../include -I$(PLAYER_INC) -lstdc++ truthlog.cc -o truthlog

# TRAPDOOR INTERFACE ########################################################

trapdoor: libtrapdoor.a

libtrapdoor.a: trapdoor.o
	ar cr $@ trapdoor.o && ranlib $@  

%.o : %.cc
	${CC} ${CCFLAGS} ${INCLUDES}  -c $< -o $@

# ADMIN #######################################################################

clean:
	rm -f *.o *~ *.a stage rtkstage manager core xs truthlog 

#dep:
#	$(MAKEDEP) -Y -I../include -I$(PLAYER_INC) -s "# Dependancies (make dep)" truthlog.cc xs.cc render.cc 2>/dev/null
#	$(RM) -f Makefile.bak
#	cd stage_objs && ${MAKE} dep
#	cd rtkstage_objs && ${MAKE} dep

#dep:
#	$(MAKEDEP) -Y -- $(CCFLAGS) -- -s "# Dependancies (make dep)" *.cc 2>/dev/null
#	sed s/"^\.\.\/"// < Makefile > Makefile.bak && mv Makefile.bak Makefile


clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak

# Dependancies (make dep)




