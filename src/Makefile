#*************************************************************************
# src/Makefile - makes the sources
# RTV
# $Id: Makefile,v 1.14 2001-05-11 21:18:11 gerkey Exp $
# ************************************************************************/

.SUFFIXES: .cc .o


MAKEDEP = makedepend

CFLAGS = -I../include -I$(PLAYERSRC)/include -I/usr/lib/g++-include -I/usr/X11R6/include -O3 -g

OBS = main.o robot.o world.o win.o image.o server.o \
	device.o playerdevice.o pioneermobiledevice.o laserdevice.o \
	visiondevice.o ptzdevice.o \
	sonardevice.o laserbeacondevice.o
LIBS = -L/usr/X11R6/lib -lX11 -lm -lpthread

clean:
	rm -f *.o *~ *.bak posreader stage

all: stage posreader

stage:  $(OBS) 
	g++ $(LIBS) $(OBS) -o stage

dep:
	$(MAKEDEP) -I../include -I$(PLAYERSRC)/include -Y -s "# DO NOT DELETE.  FOLLOWING IS FROM MAKEDEPEND" *.cc 2>/dev/null


posreader: posreader.cc
	g++ $(CFLAGS) -o posreader posreader.cc 

.cc.o: 
	g++ $(CFLAGS) -c $<

