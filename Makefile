#
# Makefile - the top level Makefile to build Stage
# RTV
# $Id: Makefile,v 1.4 2000-12-06 02:31:51 vaughan Exp $
# 

# the directory of the player source
PLAYERSRC = /usr/local/player

# install stage in this directory
INSTALL = /usr/local/stage

stage: 
	cd src; make -e PLAYERSRC=$(PLAYERSRC)  all; mv stage ../bin/

clean:
	rm -f *~ *.bak; cd bin; rm -f stage core; cd ../src; make clean; cd ../include; make clean; cd ../examples/; make clean

install: stage
	mkdir -p $(INSTALL)/bin
	install -m 755 bin/* $(INSTALL)/bin











# DO NOT DELETE


