###########################################################################
#
# File: Makefile
# Author: Andrew Howard, Richard Vaughan
# Date: 27 Apr 2001
# Desc: Top-level make file for Stage
#
# CVS info:
#  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/Makefile,v $
#  $Author: inspectorg $
#  $Revision: 1.26 $
#
###########################################################################

include Makefile.common

# INSTALL PLAYER BEFORE BUILDING STAGE OR RTKSTAGE!

##############################################################################
# Top level targets

# truthlog doesn't compile... BPG
#all: stage xs rtkstage truthlog manager
all: stage rtkstage xs manager

stage: 
	cd src && make stage && cp stage_objs/stage ../bin/

xs: 
	cd src && make xs && cp xs ../bin/

truthlog: 
	cd src && make truthlog

rtkstage:
	cd src && make rtkstage && cp rtkstage_objs/rtkstage ../bin/

manager: 
	cd src && make manager && cp manager ../bin/

dep:
	cd src &&  make dep

clean: clean_dep
	rm -f *~
	cd src && make clean
	cd include &&  make clean
	cd bin && rm -f stage rtkstage xs core
	cd examples && rm -f core

clean_dep:
	cd src && make clean_dep


###########################################################################
# Install section

INSTALL_BIN = $(INSTALL_DIR)/bin
INSTALL_BIN_FILES = bin/stage bin/xs bin/rtkstage bin/manager
INSTALL_EXAMPLES = $(INSTALL_DIR)/examples

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

