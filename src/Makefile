##############################################################################
#
# stage/src/Makefile - compiles all stage targets
# Author: Richard Vaughan, Andrew Howard
#
# $Id: Makefile,v 1.76 2002-02-07 00:45:38 rtv Exp $
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

# Flags common to stage and rtkstage
INCLUDES = -I../include -I$(PLAYER_INC)
CCFLAGS += -Wall -DREENTRANT $(ZLIB_FLAGS) $(INCLUDES) $(PLAYER_PLATFORM)
CCFLAGS += -DVERSION=\"$(VERSION)\" -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\"

# production/debug link
#LINKFLAGS = -lm -lstdc++ -pthread $(ZLIB_LIBS)

# profile link
LINKFLAGS = -pg -lm -lstdc++ -pthread $(ZLIB_LIBS)

# add the libc5 libaries if they were specified in Makefile.commin
LINKFLAGS += $(OLDLIBS)

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
	matrix.cc raytrace.cc

# stage and rtkstage objects are compiled from the same sources, but rtkstage
# defines a couple of macros to enable some optional code
#
# stage objects have a _stg.o suffix
# rtkstage objects have a _rtkstage.o suffix
STG_OBS = ${SRCS:.cc=_stg.o}
RTK_OBS = ${SRCS:.cc=_rtk.o}
HRL_OBS = ${SRCS:.cc=_hrl.o}

# use the suffix rules below to munge files with these suffixes
SUFFIXES = _stg.o _rtk.o .cc

# ALL TARGETS #############################################################

all: stage rtkstage xs manager 

# STAGE ####################################################################

stage: ${STG_OBS}
	$(LINKER) $(STG_OBS) $(LINKFLAGS) -o $@

%_stg.o : %.cc
	${CC} ${CCFLAGS} -c $< -o $@

# RTKSTAGE ##################################################################

rtkstage: LINKFLAGS += -L$(RTK_LIB) -lrtk `gtk-config --libs gtk gthread`
rtkstage: ${RTK_OBS} rtk
	$(LINKER) $(RTK_OBS) $(LINKFLAGS) -o $@ 

%_rtk.o : CCFLAGS += -DINCLUDE_RTK2 -I$(RTK_INC) `gtk-config --cflags`
%_rtk.o : %.cc
	${CC} ${CCFLAGS} -c $< -o $@

rtk:
	cd $(RTK_DIR) && ${MAKE}

# HRLSTAGE ################################################################
# hrlstage links in some HRL proprietary devices - I'm
# working on releasing these under the GPL - RTV
# hrlstage WILL NOT BUILD without the external headers and library.

hrlstage: ${HRL_OBS}
	$(LINKER) $(HRL_OBS) $(HRL_LIB) $(LINKFLAGS)  -o $@

%_hrl.o : CCFLAGS += $(HRL_HEADERS) -DHRL_HEADERS 
%_hrl.o : %.cc
	${CC} ${CCFLAGS} -c $< -o $@


# STAGE TOOLS ###############################################################

xs: CCFLAGS += -I/usr/X11R6/include 
xs: LINKFLAGS += -L/usr/X11R6/lib -lX11 -L$(PLAYER_LIB) -lplayerclient
xs : xs.cc render.cc	
	$(CC) xs.cc render.cc $(CCFLAGS) $(LINKFLAGS) -o xs

manager: manager.cc 
	$(CC) -I../include -Wall -I$(PLAYER_INC) -lstdc++ manager.cc -o manager

# truthlog doesn't compile... BPG
truthlog: truthlog.cc 
	$(CC) -I../include -I$(PLAYER_INC) -lstdc++ truthlog.cc -o truthlog

# ADMIN #######################################################################

clean:
	rm -f *.o *~ core xs truthlog

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
