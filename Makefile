###########################################################################
#
# File: Makefile
# Author: Andrew Howard, Richard Vaughan
# Date: 27 Apr 2001
# Desc: Top-level make file for Stage
#
# CVS info:
#  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/Makefile,v $
#  $Author: rtv $
#  $Revision: 1.27 $
#
#
# CONFIGURATION NOTES:
# Most user-level configs can be done here; also check src/Makefile for
# compiler options and library paths if you have trouble compiling

############################################################################
# VERSION CONFIGURATION - you probably don't need to change this
############################################################################
include VERSION

#############################################################################
# PLAYER CONFIGURATION (required) 
#############################################################################
#
# INSTALL PLAYER BEFORE BUILDING STAGE OR RTKSTAGE!
#
# Set the absolute path of your Player installation or source tree here.
# These defaults are usually OK, but you may need to change them if
# you customized your player installation

# define which version of Player you will use to build from
# this is very often the same as stage's version number:
PLAYER_VERSION = $(VERSION)
PLAYER_DIR = $(HOME)/player-$(PLAYER_VERSION)

##############################################################################
# INSTALL CONFIGURATION (optional)
##############################################################################

# Stage will be installed under the directory INSTALL_BASE-VERSION.
# Change INSTALL_BASE if you want to install somewhere else.
# You can also specify the INSTALL_BASE from the make command line, eg:
# make install -e INSTALL_BASE=~/playerstage/stage

INSTALL_BASE = $(HOME)/stage
INSTALL_DIR = $(INSTALL_BASE)-$(VERSION)

# Rtk configuration files will be installed here
INSTALL_ETC = /usr/local/etc/rtk

SRC_DIST_NAME = Stage-$(VERSION)-src
SRC_DIST_BLEEDING_NAME = Stage-Bleeding-src
BIN_DIST_NAME = Stage-$(VERSION)-i386

INSTALL_BIN = $(INSTALL_DIR)/bin
INSTALL_BIN_FILES = bin/stage bin/xs bin/rtkstage bin/manager
INSTALL_EXAMPLES = $(INSTALL_DIR)/examples

## END OF CONFIG - you shouldn't  need to change anything beyond here #######
## there are more compiler options, etc, in  src/Makefile

##############################################################################
# Build section
#

all:	stage rtkstage xs manager

stage: 	
	cd src && ${MAKE} all PLAYER_DIR=$(PLAYER_DIR) && cp stage ../bin

rtkstage: 	
	cd src && ${MAKE} rtkstage PLAYER_DIR=$(PLAYER_DIR) \
	&& cp rtkstage ../bin

xs: 	
	cd src && ${MAKE} xs PLAYER_DIR=$(PLAYER_DIR) && cp xs ../bin

manager: 	
	cd src && ${MAKE} manager PLAYER_DIR=$(PLAYER_DIR) && cp manager ../bin

dep:
	cd src && ${MAKE} dep

clean: clean_dep
	rm -f *~
	cd src && make clean
	cd include &&  make clean
	cd bin && rm -f stage rtkstage xs manager core
	cd examples && rm -f core

clean_dep:
	cd src && make clean_dep


install:
	mkdir -p $(INSTALL_EXAMPLES)
	install -m 644 examples/*.world* $(INSTALL_EXAMPLES)
	install -m 644 examples/*.m4 $(INSTALL_EXAMPLES)
	install -m 644 examples/*.pnm* $(INSTALL_EXAMPLES)
	mkdir -p $(INSTALL_DIR)/bin
	install -m 755 $(INSTALL_BIN_FILES) $(INSTALL_BIN)


###########################################################################
# Create a source distribution
# Do a make clean and remove extraneous files first.

src_dist:
	echo Building $(SRC_DIST_NAME)
	cp -R . /tmp/$(SRC_DIST_NAME)
	tar -C /tmp -cvzf $(SRC_DIST_NAME).tgz --exclude CVS --exclude '*.tgz' $(SRC_DIST_NAME)
	rm -Rf /tmp/$(SRC_DIST_NAME)

src_dist_bleeding:
	echo Building $(SRC_DIST_BLEEDING_NAME)
	cp -R . /tmp/$(SRC_DIST_BLEEDING_NAME)
	tar -C /tmp -cvzf $(SRC_DIST_BLEEDING_NAME).tgz --exclude CVS --exclude '*.tgz' $(SRC_DIST_BLEEDING_NAME)
	rm -Rf /tmp/$(SRC_DIST_BLEEDING_NAME)

###########################################################################
# Create a binary distribution
# Do a make clean and remove extraneous files first.
# Then do a make all

BIN_DIST_FILES = \
	$(BIN_DIST_NAME)/README.stage \
	$(BIN_DIST_NAME)/VERSION \
	$(BIN_DIST_NAME)/Makefile \
	$(BIN_DIST_NAME)/bin/stage \
	$(BIN_DIST_NAME)/bin/rtkstage \
	$(BIN_DIST_NAME)/etc/rtkstage.cfg  

bin_dist:
	echo Building $(BIN_DIST_NAME)
	cp -R . /tmp/$(BIN_DIST_NAME)
	tar -C /tmp -cvzf $(BIN_DIST_NAME).tgz $(BIN_DIST_FILES)
	rm -Rf /tmp/$(BIN_DIST_NAME)






