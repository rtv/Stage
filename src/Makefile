#*************************************************************************
# src/Makefile - makes the sources
# RTV
# $Id: Makefile,v 1.2 2000-11-29 04:03:51 ahoward Exp $
# ************************************************************************/

.SUFFIXES: .cc .o


MAKEDEP = makedepend

CFLAGS = -I../include -I$(PLAYERSRC)/include -I/usr/lib/g++-include -I/usr/X11R6/include -O2 -g

OBS = main.o robot.o world.o win.o image.o server.o \
	device.o laserdevice.o
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

device.o: ../include/world.h ../include/robot.h ../include/offsets.h
device.o: ../include/image.h device.hh
image.o: ../include/image.h ../include/world.h ../include/robot.h
image.o: ../include/offsets.h
laserdevice.o: ../include/world.h ../include/robot.h ../include/offsets.h
laserdevice.o: ../include/image.h laserdevice.hh device.hh
main.o: ../include/world.h ../include/robot.h ../include/offsets.h
main.o: ../include/image.h ../include/win.h
posreader.o: ../include/ports.h
robot.o: ../include/offsets.h ../include/world.h ../include/robot.h
robot.o: ../include/image.h ../include/win.h ../include/ports.h
robot.o: laserdevice.hh device.hh
server.o: ../include/world.h ../include/robot.h ../include/offsets.h
server.o: ../include/image.h ../include/win.h ../include/ports.h
win.o: ../include/win.h ../include/world.h ../include/robot.h
win.o: ../include/offsets.h ../include/image.h
world.o: ../include/world.h ../include/robot.h ../include/offsets.h
world.o: ../include/image.h ../include/win.h
