#
# $Id: Makefile,v 1.2 2004-10-02 02:19:32 rtv Exp $
#


all: manual

manual: stage.dox header.html footer.html help.txt ../src/*
	doxygen stage.dox

clean: 
	rm -rf html