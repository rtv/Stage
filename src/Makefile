#*************************************************************************
# src/Makefile - makes the sources
# RTV
# $Id: Makefile,v 1.6 2000-12-01 00:20:52 vaughan Exp $
# ************************************************************************/

.SUFFIXES: .cc .o


MAKEDEP = makedepend

CFLAGS = -I../include -I$(PLAYERSRC)/include -I/usr/lib/g++-include -I/usr/X11R6/include -O2 -g

OBS = main.o robot.o world.o win.o image.o server.o \
	device.o laserdevice.o pioneermobiledevice.o
LIBS = -L/usr/X11R6/lib -lX11 -lm -lpthread

clean:
	rm -f *.o *~ posreader stage

all: stage posreader

stage:  $(OBS) 
	g++ $(LIBS) $(OBS) -o stage

dep:
	$(MAKEDEP) -I../include -Y -s "# DO NOT DELETE.  FOLLOWING IS FROM MAKEDEPEND" *.cc 2>/dev/null


posreader: posreader.cc
	g++ $(CFLAGS) -o posreader posreader.cc 

.cc.o: 
	g++ $(CFLAGS) -c $<

# DO NOT DELETE.  FOLLOWING IS FROM MAKEDEPEND

device.o: ../include/world.h ../include/robot.h ../include/image.h
device.o: ../include/device.hh
image.o: ../include/image.h ../include/world.h ../include/robot.h
laserdevice.o: ../include/world.h ../include/robot.h ../include/image.h
laserdevice.o: ../include/laserdevice.hh ../include/device.hh
main.o: ../include/world.h ../include/robot.h ../include/image.h
main.o: ../include/win.h
posreader.o: ../include/ports.h
robot.o: ../include/world.h ../include/robot.h ../include/image.h
robot.o: ../include/win.h ../include/ports.h ../include/laserdevice.hh
robot.o: ../include/device.hh
server.o: ../include/world.h ../include/robot.h ../include/image.h
server.o: ../include/win.h ../include/ports.h
win.o: ../include/win.h ../include/world.h ../include/robot.h
win.o: ../include/image.h
world.o: ../include/world.h ../include/robot.h ../include/image.h
world.o: ../include/win.h
