#
# Makefile - the top level Makefile to build Stage
# RTV
# $Id: Makefile,v 1.7 2001-05-11 21:21:10 gerkey Exp $
# 

# the directory of the player source
PLAYERSRC = $(PWD)/../player

# install stage in this directory
INSTALL = /usr/local/stage

stage: 
	cd src; make -e PLAYERSRC=$(PLAYERSRC)  all; mv stage ../bin/
dep:
	cd src; make -e PLAYERSRC=$(PLAYERSRC) dep
clean:
	rm -f *~ *.bak; cd bin; rm -f stage core; cd ../src; make clean; cd ../include; make clean; cd ../examples/; make clean

install: stage
	mkdir -p -m 755 $(INSTALL)			
	install -m 644 README.stage $(INSTALL)	
	mkdir -p -m 755 $(INSTALL)/bin	
	install -m 755 bin/stage $(INSTALL)/bin
	install -m 755 bin/sif $(INSTALL)/bin
	mkdir -p -m 755 $(INSTALL)/examples
	install -m 644 examples/*.world $(INSTALL)/examples
	install -m 644 examples/*.pnm $(INSTALL)/examples
	install -m 644 examples/*.pos $(INSTALL)/examples

