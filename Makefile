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
#  $Revision: 1.35 $
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

all:	stage rtkstage xs manager trapdoor

stage: 	
	cd src && ${MAKE} stage 

rtkstage: 	
	cd src && ${MAKE} rtkstage

hrlstage:
	cd src && ${MAKE} hrlstage

xs: 	
	cd src && ${MAKE} xs 

hrlxs: 	
	cd src && ${MAKE} hrlxs 

manager: 	
	cd src && ${MAKE} manager 

trapdoor:
	cd src && ${MAKE} trapdoor

dep:
	cd src && ${MAKE} dep

clean: clean_dep
	rm -f *~ gmon.out 
	cd src && ${MAKE} clean
	cd include &&  ${MAKE} clean
	cd examples && rm -f core
	cd $(RTK_DIR) && ${MAKE} clean

clean_dep:
	cd src && ${MAKE} clean_dep

install:
	mkdir -p $(INSTALL_DIR)/bin
	install -m 755 $(INSTALL_BIN_FILES) $(INSTALL_BIN)
	mkdir -p $(INSTALL_LIB)
	install -m 644 src/*.a $(INSTALL_LIB)
	mkdir -p $(INSTALL_EXAMPLES)
	install -m 644 examples/*.world* $(INSTALL_EXAMPLES)
	install -m 644 examples/*.m4 $(INSTALL_EXAMPLES)
	install -m 644 examples/*.pnm* $(INSTALL_EXAMPLES)

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
#	$(BIN_DIST_NAME)/bin/rtkstage \
#	$(BIN_DIST_NAME)/etc/rtkstage.cfg  

bin_dist:
	echo Building $(BIN_DIST_NAME)
	cp -R . /tmp/$(BIN_DIST_NAME)
	mv /tmp/$(BIN_DIST_NAME)/src /tmp/$(BIN_DIST_NAME)/bin
	tar -C /tmp -cvzf $(BIN_DIST_NAME).tgz $(BIN_DIST_FILES)
	rm -Rf /tmp/$(BIN_DIST_NAME)












