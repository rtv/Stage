##############################################################################
#
# stage/src/Makefile - compiles all stage targets
# Author: Richard Vaughan, Andrew Howard
#
# $Id: Makefile,v 1.72 2002-01-26 02:41:35 rtv Exp $
#
##############################################################################
# Build type 
# choose one of these that suits your build (debug/profile/production)

# production
#CCFLAGS = -O2 
# debug
CCFLAGS = -g3 
# debug + profile
#CCFLAGS = -pg -g3 

#######################################
# Compiler selection
#
#   normally, you'll use the standard 'g++' and 'gcc' compilers. 
CC = g++

#   however, if you're running a glibc-2.2-based system (e.g., RedHat 7.x), 
#   and you want to run the resultant binaries on a glibc-2.1-based system
#   (e.g., RedHat 6.x), then use the 'i386-glibc21-linux-g++' and
#   'i386-glibc21-linux-gcc' compilers. note that you must have installed 
#   the compatibility compiler and library packages (they are included with 
#   the RedHat 7.x distro):
#     compat-libs-6.2-3
#     compat-glibc-6.2-2.1.3.2
#     compat-egcs-c++-6.2-1.1.2.14
#     compat-libstdc++-6.2-2.9.0.14
#     compat-egcs-6.2-1.1.2.14

#CC = i386-glibc21-linux-g++

# if using the old libs you also need this in order to link properly with X
#OLDLIBS = -L/usr/i386-glibc21-linux/lib

# use the same tool to link
LINKER = $(CC)

##############################################################################
# include Stage's version number
#
include ../VERSION

##############################################################################
# PLAYER CONFIGURATION (required)
# this is overridden by the top-level Makefile
# but setting sensible defaults here allows you to build from the src dir.

PLAYER_VERSION = $(VERSION)
PLAYER_DIR = $(HOME)/player-$(PLAYER_VERSION)

#############################################################################
# RTK CONFIGURATION (required for rtkstage)
# rtkstage needs the path to the rtk2 source tree - these defaults
# should work fine, as we now distribute rtk2 with stage

RTK_DIR = ../rtk2
RTK_INC = $(RTK_DIR) #/include
RTK_LIB = $(RTK_DIR) #/lib

##############################################################################
# X COLOR DATABASE CONFFIGURATION (required)
# set the path to a X11-format color database, usually rgb.txt from X distro

# this is a good guess on Linux systems
COLOR_DATABASE = /usr/X11R6/lib/X11/rgb.txt

##############################################################################
# ZLIB CONFIURATION (optional)
# Support for gzipped images; requires zlib.
# Comment out if you dont have or need zlib.

ZLIB_FLAGS = -DINCLUDE_ZLIB
ZLIB_LIBS = -lz

#############################################################################
# CONFIGURATION FOR HRL PROPRIETARY CODE - you probably don't want this

#HRL = $(HOME)/LINPHERO/pr_c++/com/hrl/pr
#HRL_HEADERS = -I$(HRL)/pherobot -I$(HRL)/abstractrobot
#HRL_SRCS = $(HRL)/descartes.cc $(HRL)/irdevice.cc

####################### DO NOT CHANGE BELOW HERE #############################

PLAYER_INC = $(PLAYER_DIR)/include
PLAYER_LIB = $(PLAYER_DIR)/lib

# Flags common to all targets
#
INCLUDES = -I../include -I$(PLAYER_INC)
CCFLAGS += -Wall -DREENTRANT $(ZLIB_FLAGS) $(INCLUDES) $(HRL_HEADERS) 
CCFLAGS += -DVERSION=\"$(VERSION)\" -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\"

# production/debug link
LINKFLAGS = -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
# profile link
#LINKFLAGS = -pg -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
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
	matrix.cc raytrace.cc $(HRL_SRCS)

# stage and rtkstage objects are compiled from the same sources, but rtkstage
# defines a couple of macros to enable some optional code
#
# stage objects have a _stg.o suffix
# rtkstage objects have a _rtkstage.o suffix
STG_OBS = ${SRCS:.cc=_stg.o}
RTK_OBS = ${SRCS:.cc=_rtk.o}

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
	cd $(RTK_DIR) && make


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
#	cd stage_objs && make dep
#	cd rtkstage_objs && make dep

#dep:
#	$(MAKEDEP) -Y -- $(CCFLAGS) -- -s "# Dependancies (make dep)" *.cc 2>/dev/null
#	sed s/"^\.\.\/"// < Makefile > Makefile.bak && mv Makefile.bak Makefile


clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak

# Dependancies (make dep)
