#
# Makefile - the top level Makefile to build Stage
# RTV
# $Id: Makefile,v 1.3.2.3 2001-04-02 18:41:45 ahoward Exp $
# 

# the directory of the player source
PLAYERSRC = /usr/local/player

# install stage in these directories
INSTALL = /usr/local/stage
INSTALL_BIN = $(INSTALL)/bin
INSTALL_ETC = /usr/local/etc/rtk
INSTALL_BIN_FILES = bin/stage bin/rtk_stage
INSTALL_ETC_FILES = etc/rtk_stage.cfg

stage: 
	cd src; make clean; make stage; mv stage ../bin/

rtk_stage:
	cd src; make clean; make rtk_stage; mv rtk_stage ../bin/

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


