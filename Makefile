#
# Makefile - the top level Makefile to build Stage
# RTV
# $Id: Makefile,v 1.3.2.4 2001-04-27 20:23:46 ahoward Exp $
# 

# the directory of the player source
PLAYER_DIR = /usr/local/player1.1beta
#PLAYER_DIR = ../../player
RTK_DIR = ../../rtk

# install stage in these directories
INSTALL = /usr/local/stage
INSTALL_BIN = $(INSTALL)/bin
INSTALL_ETC = /usr/local/etc/rtk
INSTALL_BIN_FILES = bin/stage bin/rtk_stage
INSTALL_ETC_FILES = etc/rtk_stage.cfg

stage: 
	cd src; make stage -e PLAYER_DIR=$(PLAYER_DIR); cp stage ../bin/

rtk_stage:
	cd src; make rtk_stage -e PLAYER_DIR=$(PLAYER_DIR) -e RTK_DIR=$(RTK_DIR); \
	cp rtk_stage ../bin/

dep:
	cd src; make dep

clean:
	rm -f *~
	cd bin; rm -f stage rtk_stage core
	cd src; make clean 
	cd include; make clean
	cd examples; make clean

install:
	mkdir -p $(INSTALL_BIN)
	install -m 755 $(INSTALL_BIN_FILES) $(INSTALL_BIN)
	mkdir -p $(INSTALL_ETC)
	install -m 755 $(INSTALL_ETC_FILES) $(INSTALL_ETC)



# DO NOT DELETE


