###########################################################################
#
# File: Makefile
# Author: Andrew Howard, Richard Vaughan
# Date: 27 Apr 2001
# Desc: Top-level make file for Stage
#
# CVS info:
#  $Source: /home/tcollett/stagecvs/playerstage-cvs/code/stage/Makefile,v $
#  $Author: gerkey $
#  $Revision: 1.43 $
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

all:	stage tools

stage: 	
	cd src && ${MAKE} stage 

tools:
	cd src $$ ${MAKE} tools

dep:
	cd src && ${MAKE} dep

clean: clean_dep
	rm -f *~ gmon.out core core.*
	cd src && ${MAKE} clean
	cd include &&  ${MAKE} clean
	cd worlds && rm -f core
	cd $(RTK_DIR) && ${MAKE} -f Makefile.manual clean

clean_dep:
	cd src && ${MAKE} clean_dep

install: all
	mkdir -p $(INSTALL_DIR)/bin
	install -m 755 src/stage $(INSTALL_BIN)
	mkdir -p $(INSTALL_WORLDS)
	install -m 644 worlds/*.world $(INSTALL_WORLDS)
	install -m 644 worlds/*.inc $(INSTALL_WORLDS)
	install -m 644 worlds/*.pnm* $(INSTALL_WORLDS)
	mkdir -p $(INSTALL_DOC)
	install -m 644 doc/* $(INSTALL_DOC)
	mkdir -p $(INSTALL_TOOLS)
	install -m 755 tools/* $(INSTALL_TOOLS)

uninstall:
	rm -f $(INSTALL_BIN)/stage
	rmdir --ignore-fail-on-non-empty $(INSTALL_BIN)
	rm -f $(INSTALL_WORLDS)/*.world
	rm -f $(INSTALL_WORLDS)/*.inc 
	rm -f $(INSTALL_WORLDS)/*.pnm* 
	rmdir --ignore-fail-on-non-empty $(INSTALL_WORLDS)
	rm -f $(INSTALL_DOC)/* 
	rmdir --ignore-fail-on-non-empty $(INSTALL_DOC)
	rm -f $(INSTALL_TOOLS)/* 
	rmdir --ignore-fail-on-non-empty $(INSTALL_TOOLS)
	rmdir --ignore-fail-on-non-empty $(INSTALL_DIR)

fresh: clean
	${MAKE} dep all

###########################################################################
# Create a source distribution
# Do a make clean and remove extraneous files first.

src_dist: clean
	cd doc && make ps
	echo Building $(SRC_DIST_NAME)
	cp -R . /tmp/$(SRC_DIST_NAME)
	tar -C /tmp -cvzf $(SRC_DIST_NAME).tgz --exclude CVS --exclude '*.tgz' --exclude "*/doc/Makefile" --exclude "*/doc/*.aux" --exclude "*/doc/*.log" --exclude "*/doc/tex" --exclude "*/doc/*.toc" --exclude "*/doc/*.eps" --exclude "*/doc/*.jpg" --exclude "*/doc/*.tex" --exclude "*/doc/*.bbl" --exclude "*/doc/*.bib" --exclude "*/doc/*.dvi" $(SRC_DIST_NAME)
	rm -Rf /tmp/$(SRC_DIST_NAME)

# because i can't remember to type src_dist - BPG
distro: src_dist

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












