###########################################################################
#
# File: Makefile
# Author: Andrew Howard, Richard Vaughan
# Date: 27 Apr 2001
# Desc: Top-level make file for Stage
#
# CVS info:
#  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/Makefile,v $
#  $Author: gsibley $
#  $Revision: 1.34 $
#
# Note: All normal user configurations are in Makefile.common - you
# probably don't need to change this file
#
############################################################################

# include the configuration file
include Makefile.common

##############################################################################
# Build section
#

all:	stage rtkstage xs manager

stage: 	
	cd src && ${MAKE} stage PLAYER_DIR=$(PLAYER_DIR) \
	&& cp stage ../bin

rtkstage: 	
	cd src && ${MAKE} rtkstage PLAYER_DIR=$(PLAYER_DIR) \
	&& cp rtkstage ../bin

hrlstage: 
	cd hrl && make && cd ..	
	cd src && ${MAKE} hrlstage PLAYER_DIR=$(PLAYER_DIR) \
	&& cp hrlstage ../bin

xs: 	
	cd src && ${MAKE} xs PLAYER_DIR=$(PLAYER_DIR) && cp xs ../bin

manager: 	
	cd src && ${MAKE} manager PLAYER_DIR=$(PLAYER_DIR) && cp manager ../bin

dep:
	cd src && ${MAKE} dep

clean: clean_dep
	rm -f *~ gmon.out 
	cd src && ${MAKE} clean
	cd include &&  ${MAKE} clean
	cd bin && rm -f stage rtkstage hrlstage xs manager core
	cd examples && rm -f core
	cd ../rtk2 && ${MAKE} clean
#	cd hrl && ${MAKE} clean

clean_dep:
	cd src && ${MAKE} clean_dep


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












