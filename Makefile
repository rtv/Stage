#
# Makefile - the top level Makefile to build Stage
# RTV
# $Id: Makefile,v 1.3.2.2 2001-03-13 23:06:10 ahoward Exp $
# 

# the directory of the player source
PLAYERSRC = /usr/local/player

# install stage in this directory
INSTALL = /usr/local/stage

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

install: stage
	mkdir -p $(INSTALL)/bin
	install -m 755 bin/* $(INSTALL)/bin



# DO NOT DELETE


