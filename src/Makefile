
##############################################################################
# This file should be called from the top-level make file, 
# which will set the values of the key directories


##############################################################################
# These are DEFAULT values -- they will get overridden by the top-level Makefile
# They are just here so you can compile stuff while testing new code.

VERSION=$(shell awk '{print substr($$3,2,length($$3)-2);}' ../VERSION)
PLAYER_DIR = /usr/local/player$(VERSION)


##############################################################################
# Support for gzipped images; requires zlib.
# Comment out if you dont have/need zlib.

ZLIB_FLAGS = -DINCLUDE_ZLIB
ZLIB_LIBS = -lz


##############################################################################
# These are the key paths needed for the build

PLAYER_INC = $(PLAYER_DIR)/include
PLAYER_LIB = $(PLAYER_DIR)/lib
RTK_DIR = ../rtk
RTK_INC = $(RTK_DIR)/include
RTK_LIB = $(RTK_DIR)/lib


##############################################################################
# Build section

# Objects common to all targets
#
OBJS = world.o world_load.o image.o \
		entity.o entityfactory.o \
		playerserver.o playerdevice.o \
		pioneermobiledevice.o laserdevice.o \
		visiondevice.o ptzdevice.o sonardevice.o \
		laserbeacondevice.o broadcastdevice.o miscdevice.o \
		boxobstacle.o laserbeacon.o visionbeacon.o \
		usc_pioneer.o

# Flags common to all targets
#
CCFLAGS = -g3 -Wall -I../include -I$(PLAYER_INC) $(ZLIB_FLAGS)
LINKFLAGS = -lm -lstdc++ -lpthread  -L$(PLAYER_LIB) -lplayerclient $(ZLIB_LIBS)

# Standard tools
#
CC := g++
LINKER = gcc
MAKEDEP = makedepend

.SUFFIXES: .cc .o

##############################################################################
# Targets

# Target stage with no GUI
#
stage : OBJS += main.o
stage : $(OBJS) main.o 
	${LINKER} $(OBJS) $(LINKFLAGS) -o stage

# Target stage with Xlib GUI
#
xstage : CCFLAGS += -DINCLUDE_XGUI -I/usr/X11R6/include
xstage : OBJS += main.o xgui.o
xstage : $(OBJS) main.o xgui.o 
	${LINKER} $(OBJS) $(LINKFLAGS) -L/usr/X11R6/lib -lX11 -o xstage


# Target stage with RTK GUI
#
rtkstage : OBJS += rtk_main.o
rtkstage : CCFLAGS += -DINCLUDE_RTK -I$(RTK_INC) `gtk-config --cflags`
rtkstage : LINKFLAGS += -L$(RTK_LIB) -lrtk `gtk-config --libs gtk gthread`
rtkstage : $(OBJS) rtk_main.o 
	${LINKER} $(OBJS) $(LINKFLAGS) -o rtkstage

clean:
	rm -f *.o *~ core

dep:
	$(MAKEDEP) -Y -I../include -I$(PLAYER_INC) -s "# Dependancies (make dep)" *.cc 2>/dev/null

%.o : %.cc
	${CC} ${CCFLAGS} -c $< -o $@


##############################################################################
# Dependancies (make dep)


boxobstacle..o: ../include/boxobstacle.hh ../include/entity.hh
boxobstacle..o: ../include/image.h ../include/stage_types.hh
boxobstacle..o: ../include/world.hh /usr/include/alloca.h
boxobstacle..o: /usr/include/assert.h /usr/include/bits/endian.h
boxobstacle..o: /usr/include/bits/huge_val.h /usr/include/bits/initspin.h
boxobstacle..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
boxobstacle..o: /usr/include/bits/nan.h /usr/include/bits/pthreadtypes.h
boxobstacle..o: /usr/include/bits/sched.h /usr/include/bits/select.h
boxobstacle..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
boxobstacle..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
boxobstacle..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
boxobstacle..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
boxobstacle..o: /usr/include/bits/wordsize.h /usr/include/endian.h
boxobstacle..o: /usr/include/features.h /usr/include/_G_config.h
boxobstacle..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
boxobstacle..o: /usr/include/libio.h /usr/include/math.h
boxobstacle..o: /usr/include/pthread.h /usr/include/sched.h
boxobstacle..o: /usr/include/signal.h /usr/include/stdint.h
boxobstacle..o: /usr/include/stdio.h /usr/include/stdlib.h
boxobstacle..o: /usr/include/string.h /usr/include/sys/cdefs.h
boxobstacle..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
boxobstacle..o: /usr/include/sys/types.h /usr/include/time.h
boxobstacle..o: /usr/include/wchar.h /usr/include/xlocale.h
boxobstacle..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
boxobstacle..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
broadcastdevice..o: ../include/broadcastdevice.hh ../include/entity.hh
broadcastdevice..o: ../include/image.h ../include/playerdevice.hh
broadcastdevice..o: ../include/stage_types.hh ../include/world.hh
broadcastdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
broadcastdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
broadcastdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
broadcastdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
broadcastdevice..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
broadcastdevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
broadcastdevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
broadcastdevice..o: /usr/include/bits/posix2_lim.h
broadcastdevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
broadcastdevice..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
broadcastdevice..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
broadcastdevice..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
broadcastdevice..o: /usr/include/bits/time.h /usr/include/bits/types.h
broadcastdevice..o: /usr/include/bits/waitflags.h
broadcastdevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
broadcastdevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
broadcastdevice..o: /usr/include/endian.h /usr/include/features.h
broadcastdevice..o: /usr/include/_G_config.h /usr/include/gconv.h
broadcastdevice..o: /usr/include/gnu/stubs.h /usr/include/libio.h
broadcastdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
broadcastdevice..o: /usr/include/math.h /usr/include/netinet/in.h
broadcastdevice..o: /usr/include/pthread.h /usr/include/sched.h
broadcastdevice..o: /usr/include/signal.h /usr/include/stdint.h
broadcastdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
broadcastdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
broadcastdevice..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
broadcastdevice..o: /usr/include/sys/types.h /usr/include/time.h
broadcastdevice..o: /usr/include/wchar.h /usr/include/xlocale.h
broadcastdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
broadcastdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
broadcastdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
broadcastdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
broadcastdevice..o: /usr/local/player1.1.1beta/include/messages.h
broadcastdevice..o: /usr/local/player1.1.1beta/include/playercommon.h
broadcastdevice..o: /usr/local/player1.1.1beta/include/stage.h
entityfactory..o: ../include/boxobstacle.hh ../include/entityfactory.hh
entityfactory..o: ../include/entity.hh ../include/laserbeacon.hh
entityfactory..o: ../include/stage_types.hh ../include/usc_pioneer.hh
entityfactory..o: ../include/visionbeacon.hh /usr/include/assert.h
entityfactory..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
entityfactory..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
entityfactory..o: /usr/include/bits/nan.h /usr/include/endian.h
entityfactory..o: /usr/include/features.h /usr/include/gnu/stubs.h
entityfactory..o: /usr/include/math.h /usr/include/string.h
entityfactory..o: /usr/include/sys/cdefs.h /usr/include/xlocale.h
entityfactory..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
entity..o: ../include/entity.hh ../include/stage_types.hh
entity..o: /usr/include/alloca.h /usr/include/assert.h
entity..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
entity..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
entity..o: /usr/include/bits/nan.h /usr/include/bits/pthreadtypes.h
entity..o: /usr/include/bits/sched.h /usr/include/bits/select.h
entity..o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
entity..o: /usr/include/bits/time.h /usr/include/bits/types.h
entity..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
entity..o: /usr/include/bits/wchar.h /usr/include/endian.h
entity..o: /usr/include/features.h /usr/include/_G_config.h
entity..o: /usr/include/gconv.h /usr/include/gnu/stubs.h /usr/include/libio.h
entity..o: /usr/include/math.h /usr/include/stdio.h /usr/include/stdlib.h
entity..o: /usr/include/string.h /usr/include/sys/cdefs.h
entity..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
entity..o: /usr/include/sys/types.h /usr/include/time.h /usr/include/wchar.h
entity..o: /usr/include/xlocale.h
entity..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
entity..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
image..o: ../include/image.h /usr/include/alloca.h /usr/include/bits/endian.h
image..o: /usr/include/bits/huge_val.h /usr/include/bits/mathcalls.h
image..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
image..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
image..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
image..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
image..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
image..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
image..o: /usr/include/endian.h /usr/include/features.h
image..o: /usr/include/g++-3/fstream.h /usr/include/g++-3/iostream.h
image..o: /usr/include/g++-3/streambuf.h /usr/include/_G_config.h
image..o: /usr/include/gconv.h /usr/include/gnu/stubs.h /usr/include/libio.h
image..o: /usr/include/math.h /usr/include/stdio.h /usr/include/stdlib.h
image..o: /usr/include/string.h /usr/include/sys/cdefs.h
image..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
image..o: /usr/include/sys/types.h /usr/include/time.h /usr/include/wchar.h
image..o: /usr/include/xlocale.h
image..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
image..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
laserbeacondevice..o: ../include/entity.hh ../include/image.h
laserbeacondevice..o: ../include/laserbeacondevice.hh
laserbeacondevice..o: ../include/laserdevice.hh ../include/playerdevice.hh
laserbeacondevice..o: ../include/stage_types.hh ../include/world.hh
laserbeacondevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
laserbeacondevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
laserbeacondevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
laserbeacondevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
laserbeacondevice..o: /usr/include/bits/initspin.h
laserbeacondevice..o: /usr/include/bits/local_lim.h
laserbeacondevice..o: /usr/include/bits/mathcalls.h
laserbeacondevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
laserbeacondevice..o: /usr/include/bits/posix1_lim.h
laserbeacondevice..o: /usr/include/bits/posix2_lim.h
laserbeacondevice..o: /usr/include/bits/pthreadtypes.h
laserbeacondevice..o: /usr/include/bits/sched.h /usr/include/bits/select.h
laserbeacondevice..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
laserbeacondevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
laserbeacondevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
laserbeacondevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
laserbeacondevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
laserbeacondevice..o: /usr/include/bits/wordsize.h
laserbeacondevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
laserbeacondevice..o: /usr/include/features.h /usr/include/_G_config.h
laserbeacondevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
laserbeacondevice..o: /usr/include/libio.h /usr/include/limits.h
laserbeacondevice..o: /usr/include/linux/limits.h /usr/include/math.h
laserbeacondevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
laserbeacondevice..o: /usr/include/sched.h /usr/include/signal.h
laserbeacondevice..o: /usr/include/stdint.h /usr/include/stdio.h
laserbeacondevice..o: /usr/include/stdlib.h /usr/include/string.h
laserbeacondevice..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
laserbeacondevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
laserbeacondevice..o: /usr/include/time.h /usr/include/wchar.h
laserbeacondevice..o: /usr/include/xlocale.h
laserbeacondevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
laserbeacondevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
laserbeacondevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
laserbeacondevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
laserbeacondevice..o: /usr/local/player1.1.1beta/include/messages.h
laserbeacondevice..o: /usr/local/player1.1.1beta/include/playercommon.h
laserbeacondevice..o: /usr/local/player1.1.1beta/include/stage.h
laserbeacon..o: ../include/entity.hh ../include/image.h
laserbeacon..o: ../include/laserbeacon.hh ../include/stage_types.hh
laserbeacon..o: ../include/world.hh /usr/include/alloca.h
laserbeacon..o: /usr/include/assert.h /usr/include/bits/endian.h
laserbeacon..o: /usr/include/bits/huge_val.h /usr/include/bits/initspin.h
laserbeacon..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
laserbeacon..o: /usr/include/bits/nan.h /usr/include/bits/pthreadtypes.h
laserbeacon..o: /usr/include/bits/sched.h /usr/include/bits/select.h
laserbeacon..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
laserbeacon..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
laserbeacon..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
laserbeacon..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
laserbeacon..o: /usr/include/bits/wordsize.h /usr/include/endian.h
laserbeacon..o: /usr/include/features.h /usr/include/_G_config.h
laserbeacon..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
laserbeacon..o: /usr/include/libio.h /usr/include/math.h
laserbeacon..o: /usr/include/pthread.h /usr/include/sched.h
laserbeacon..o: /usr/include/signal.h /usr/include/stdint.h
laserbeacon..o: /usr/include/stdio.h /usr/include/stdlib.h
laserbeacon..o: /usr/include/string.h /usr/include/sys/cdefs.h
laserbeacon..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
laserbeacon..o: /usr/include/sys/types.h /usr/include/time.h
laserbeacon..o: /usr/include/wchar.h /usr/include/xlocale.h
laserbeacon..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
laserbeacon..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
laserdevice..o: ../include/entity.hh ../include/image.h
laserdevice..o: ../include/laserdevice.hh ../include/playerdevice.hh
laserdevice..o: ../include/stage_types.hh ../include/world.hh
laserdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
laserdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
laserdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
laserdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
laserdevice..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
laserdevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
laserdevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
laserdevice..o: /usr/include/bits/posix2_lim.h
laserdevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
laserdevice..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
laserdevice..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
laserdevice..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
laserdevice..o: /usr/include/bits/time.h /usr/include/bits/types.h
laserdevice..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
laserdevice..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
laserdevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
laserdevice..o: /usr/include/features.h /usr/include/_G_config.h
laserdevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
laserdevice..o: /usr/include/libio.h /usr/include/limits.h
laserdevice..o: /usr/include/linux/limits.h /usr/include/math.h
laserdevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
laserdevice..o: /usr/include/sched.h /usr/include/signal.h
laserdevice..o: /usr/include/stdint.h /usr/include/stdio.h
laserdevice..o: /usr/include/stdlib.h /usr/include/string.h
laserdevice..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
laserdevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
laserdevice..o: /usr/include/time.h /usr/include/wchar.h
laserdevice..o: /usr/include/xlocale.h
laserdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
laserdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
laserdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
laserdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
laserdevice..o: /usr/local/player1.1.1beta/include/messages.h
laserdevice..o: /usr/local/player1.1.1beta/include/playercommon.h
laserdevice..o: /usr/local/player1.1.1beta/include/stage.h
main..o: ../include/entity.hh ../include/image.h ../include/stage_types.hh
main..o: ../include/world.hh /usr/include/alloca.h
main..o: /usr/include/asm/sigcontext.h /usr/include/assert.h
main..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
main..o: /usr/include/bits/environments.h /usr/include/bits/huge_val.h
main..o: /usr/include/bits/initspin.h /usr/include/bits/mathcalls.h
main..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
main..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
main..o: /usr/include/bits/sched.h /usr/include/bits/select.h
main..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
main..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
main..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
main..o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
main..o: /usr/include/bits/time.h /usr/include/bits/types.h
main..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
main..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
main..o: /usr/include/endian.h /usr/include/features.h
main..o: /usr/include/_G_config.h /usr/include/gconv.h /usr/include/getopt.h
main..o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/math.h
main..o: /usr/include/pthread.h /usr/include/sched.h /usr/include/signal.h
main..o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
main..o: /usr/include/string.h /usr/include/sys/cdefs.h
main..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
main..o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
main..o: /usr/include/time.h /usr/include/ucontext.h /usr/include/unistd.h
main..o: /usr/include/wchar.h /usr/include/xlocale.h
main..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
main..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h ../VERSION
miscdevice..o: ../include/entity.hh ../include/image.h
miscdevice..o: ../include/miscdevice.hh ../include/playerdevice.hh
miscdevice..o: ../include/stage_types.hh ../include/world.hh
miscdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
miscdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
miscdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
miscdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
miscdevice..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
miscdevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
miscdevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
miscdevice..o: /usr/include/bits/posix2_lim.h /usr/include/bits/pthreadtypes.h
miscdevice..o: /usr/include/bits/sched.h /usr/include/bits/select.h
miscdevice..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
miscdevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
miscdevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
miscdevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
miscdevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
miscdevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
miscdevice..o: /usr/include/endian.h /usr/include/features.h
miscdevice..o: /usr/include/_G_config.h /usr/include/gconv.h
miscdevice..o: /usr/include/gnu/stubs.h /usr/include/libio.h
miscdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
miscdevice..o: /usr/include/math.h /usr/include/netinet/in.h
miscdevice..o: /usr/include/pthread.h /usr/include/sched.h
miscdevice..o: /usr/include/signal.h /usr/include/stdint.h
miscdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
miscdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
miscdevice..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
miscdevice..o: /usr/include/sys/types.h /usr/include/time.h
miscdevice..o: /usr/include/wchar.h /usr/include/xlocale.h
miscdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
miscdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
miscdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
miscdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
miscdevice..o: /usr/local/player1.1.1beta/include/messages.h
miscdevice..o: /usr/local/player1.1.1beta/include/playercommon.h
miscdevice..o: /usr/local/player1.1.1beta/include/stage.h
pioneermobiledevice..o: ../include/entity.hh ../include/image.h
pioneermobiledevice..o: ../include/pioneermobiledevice.hh
pioneermobiledevice..o: ../include/playerdevice.hh ../include/stage_types.hh
pioneermobiledevice..o: ../include/world.hh /usr/include/alloca.h
pioneermobiledevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
pioneermobiledevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
pioneermobiledevice..o: /usr/include/bits/endian.h
pioneermobiledevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
pioneermobiledevice..o: /usr/include/bits/initspin.h
pioneermobiledevice..o: /usr/include/bits/local_lim.h
pioneermobiledevice..o: /usr/include/bits/mathcalls.h
pioneermobiledevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
pioneermobiledevice..o: /usr/include/bits/posix1_lim.h
pioneermobiledevice..o: /usr/include/bits/posix2_lim.h
pioneermobiledevice..o: /usr/include/bits/pthreadtypes.h
pioneermobiledevice..o: /usr/include/bits/sched.h /usr/include/bits/select.h
pioneermobiledevice..o: /usr/include/bits/sigset.h
pioneermobiledevice..o: /usr/include/bits/sigthread.h
pioneermobiledevice..o: /usr/include/bits/sockaddr.h
pioneermobiledevice..o: /usr/include/bits/socket.h
pioneermobiledevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
pioneermobiledevice..o: /usr/include/bits/types.h
pioneermobiledevice..o: /usr/include/bits/waitflags.h
pioneermobiledevice..o: /usr/include/bits/waitstatus.h
pioneermobiledevice..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
pioneermobiledevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
pioneermobiledevice..o: /usr/include/features.h /usr/include/_G_config.h
pioneermobiledevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
pioneermobiledevice..o: /usr/include/libio.h /usr/include/limits.h
pioneermobiledevice..o: /usr/include/linux/limits.h /usr/include/math.h
pioneermobiledevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
pioneermobiledevice..o: /usr/include/sched.h /usr/include/signal.h
pioneermobiledevice..o: /usr/include/stdint.h /usr/include/stdio.h
pioneermobiledevice..o: /usr/include/stdlib.h /usr/include/string.h
pioneermobiledevice..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
pioneermobiledevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
pioneermobiledevice..o: /usr/include/time.h /usr/include/wchar.h
pioneermobiledevice..o: /usr/include/xlocale.h
pioneermobiledevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
pioneermobiledevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
pioneermobiledevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
pioneermobiledevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
pioneermobiledevice..o: /usr/local/player1.1.1beta/include/messages.h
pioneermobiledevice..o: /usr/local/player1.1.1beta/include/playercommon.h
pioneermobiledevice..o: /usr/local/player1.1.1beta/include/stage.h
playerdevice..o: ../include/entity.hh ../include/image.h
playerdevice..o: ../include/playerdevice.hh ../include/playerserver.hh
playerdevice..o: ../include/stage_types.hh ../include/world.hh
playerdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
playerdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
playerdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
playerdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
playerdevice..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
playerdevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
playerdevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
playerdevice..o: /usr/include/bits/posix2_lim.h
playerdevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
playerdevice..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
playerdevice..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
playerdevice..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
playerdevice..o: /usr/include/bits/time.h /usr/include/bits/types.h
playerdevice..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
playerdevice..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
playerdevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
playerdevice..o: /usr/include/features.h /usr/include/_G_config.h
playerdevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
playerdevice..o: /usr/include/libio.h /usr/include/limits.h
playerdevice..o: /usr/include/linux/limits.h /usr/include/math.h
playerdevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
playerdevice..o: /usr/include/sched.h /usr/include/signal.h
playerdevice..o: /usr/include/stdint.h /usr/include/stdio.h
playerdevice..o: /usr/include/stdlib.h /usr/include/string.h
playerdevice..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
playerdevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
playerdevice..o: /usr/include/sys/types.h /usr/include/time.h
playerdevice..o: /usr/include/wchar.h /usr/include/xlocale.h
playerdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
playerdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
playerdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
playerdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
playerdevice..o: /usr/local/player1.1.1beta/include/messages.h
playerdevice..o: /usr/local/player1.1.1beta/include/playercommon.h
playerdevice..o: /usr/local/player1.1.1beta/include/stage.h
playerserver..o: ../include/entity.hh ../include/image.h
playerserver..o: ../include/playerserver.hh ../include/stage_types.hh
playerserver..o: ../include/world.hh /usr/include/alloca.h
playerserver..o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
playerserver..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
playerserver..o: /usr/include/assert.h /usr/include/bits/byteswap.h
playerserver..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
playerserver..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
playerserver..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
playerserver..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
playerserver..o: /usr/include/bits/ipc.h /usr/include/bits/local_lim.h
playerserver..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
playerserver..o: /usr/include/bits/mman.h /usr/include/bits/nan.h
playerserver..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
playerserver..o: /usr/include/bits/posix_opt.h
playerserver..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/resource.h
playerserver..o: /usr/include/bits/sched.h /usr/include/bits/select.h
playerserver..o: /usr/include/bits/sem.h /usr/include/bits/sigaction.h
playerserver..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
playerserver..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
playerserver..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
playerserver..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
playerserver..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
playerserver..o: /usr/include/bits/termios.h /usr/include/bits/time.h
playerserver..o: /usr/include/bits/types.h /usr/include/bits/uio.h
playerserver..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
playerserver..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
playerserver..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
playerserver..o: /usr/include/errno.h /usr/include/fcntl.h
playerserver..o: /usr/include/features.h /usr/include/g++-3/iomanip.h
playerserver..o: /usr/include/g++-3/iostream.h /usr/include/g++-3/streambuf.h
playerserver..o: /usr/include/g++-3/strfile.h /usr/include/g++-3/strstream.h
playerserver..o: /usr/include/_G_config.h /usr/include/gconv.h
playerserver..o: /usr/include/getopt.h /usr/include/gnu/stubs.h
playerserver..o: /usr/include/libio.h /usr/include/limits.h
playerserver..o: /usr/include/linux/errno.h /usr/include/linux/limits.h
playerserver..o: /usr/include/math.h /usr/include/netinet/in.h
playerserver..o: /usr/include/pthread.h /usr/include/sched.h
playerserver..o: /usr/include/signal.h /usr/include/stdint.h
playerserver..o: /usr/include/stdio.h /usr/include/stdlib.h
playerserver..o: /usr/include/string.h /usr/include/sys/cdefs.h
playerserver..o: /usr/include/sys/ipc.h /usr/include/sys/mman.h
playerserver..o: /usr/include/sys/resource.h /usr/include/sys/select.h
playerserver..o: /usr/include/sys/sem.h /usr/include/sys/socket.h
playerserver..o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
playerserver..o: /usr/include/sys/ttydefaults.h /usr/include/sys/types.h
playerserver..o: /usr/include/sys/ucontext.h /usr/include/sys/uio.h
playerserver..o: /usr/include/sys/wait.h /usr/include/termios.h
playerserver..o: /usr/include/time.h /usr/include/ucontext.h
playerserver..o: /usr/include/unistd.h /usr/include/wchar.h
playerserver..o: /usr/include/xlocale.h
playerserver..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
playerserver..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
playerserver..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
playerserver..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
playerserver..o: /usr/local/player1.1.1beta/include/messages.h
playerserver..o: /usr/local/player1.1.1beta/include/playercommon.h
playerserver..o: /usr/local/player1.1.1beta/include/stage.h
posreader..o: ../include/ports.h /usr/include/alloca.h
posreader..o: /usr/include/arpa/inet.h /usr/include/asm/errno.h
posreader..o: /usr/include/asm/sigcontext.h /usr/include/asm/socket.h
posreader..o: /usr/include/asm/sockios.h /usr/include/bits/byteswap.h
posreader..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
posreader..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
posreader..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
posreader..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
posreader..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
posreader..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
posreader..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
posreader..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
posreader..o: /usr/include/bits/sched.h /usr/include/bits/select.h
posreader..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
posreader..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
posreader..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
posreader..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
posreader..o: /usr/include/bits/socket.h /usr/include/bits/stat.h
posreader..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
posreader..o: /usr/include/bits/types.h /usr/include/bits/uio.h
posreader..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
posreader..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
posreader..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
posreader..o: /usr/include/errno.h /usr/include/fcntl.h
posreader..o: /usr/include/features.h /usr/include/g++-3/iostream.h
posreader..o: /usr/include/g++-3/streambuf.h /usr/include/_G_config.h
posreader..o: /usr/include/gconv.h /usr/include/getopt.h
posreader..o: /usr/include/gnu/stubs.h /usr/include/libio.h
posreader..o: /usr/include/limits.h /usr/include/linux/errno.h
posreader..o: /usr/include/linux/limits.h /usr/include/math.h
posreader..o: /usr/include/netinet/in.h /usr/include/pthread.h
posreader..o: /usr/include/sched.h /usr/include/signal.h /usr/include/stdint.h
posreader..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
posreader..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
posreader..o: /usr/include/sys/socket.h /usr/include/sys/stat.h
posreader..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
posreader..o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
posreader..o: /usr/include/sys/uio.h /usr/include/time.h
posreader..o: /usr/include/ucontext.h /usr/include/unistd.h
posreader..o: /usr/include/values.h /usr/include/wchar.h
posreader..o: /usr/include/xlocale.h
posreader..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/float.h
posreader..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
posreader..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
posreader..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
posreader..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
ptzdevice..o: ../include/entity.hh ../include/image.h
ptzdevice..o: ../include/playerdevice.hh ../include/ptzdevice.hh
ptzdevice..o: ../include/stage_types.hh ../include/world.hh
ptzdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
ptzdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
ptzdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
ptzdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
ptzdevice..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
ptzdevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
ptzdevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
ptzdevice..o: /usr/include/bits/posix2_lim.h /usr/include/bits/pthreadtypes.h
ptzdevice..o: /usr/include/bits/sched.h /usr/include/bits/select.h
ptzdevice..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
ptzdevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
ptzdevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
ptzdevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
ptzdevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
ptzdevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
ptzdevice..o: /usr/include/endian.h /usr/include/features.h
ptzdevice..o: /usr/include/_G_config.h /usr/include/gconv.h
ptzdevice..o: /usr/include/gnu/stubs.h /usr/include/libio.h
ptzdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
ptzdevice..o: /usr/include/math.h /usr/include/netinet/in.h
ptzdevice..o: /usr/include/pthread.h /usr/include/sched.h
ptzdevice..o: /usr/include/signal.h /usr/include/stdint.h /usr/include/stdio.h
ptzdevice..o: /usr/include/stdlib.h /usr/include/string.h
ptzdevice..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
ptzdevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
ptzdevice..o: /usr/include/time.h /usr/include/wchar.h /usr/include/xlocale.h
ptzdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
ptzdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
ptzdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
ptzdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
ptzdevice..o: /usr/local/player1.1.1beta/include/messages.h
ptzdevice..o: /usr/local/player1.1.1beta/include/playercommon.h
ptzdevice..o: /usr/local/player1.1.1beta/include/stage.h
rtk_main..o: ../include/entity.hh ../include/image.h ../include/stage_types.hh
rtk_main..o: ../include/world.hh /usr/include/alloca.h /usr/include/assert.h
rtk_main..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
rtk_main..o: /usr/include/bits/initspin.h /usr/include/bits/mathcalls.h
rtk_main..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
rtk_main..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
rtk_main..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
rtk_main..o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
rtk_main..o: /usr/include/bits/time.h /usr/include/bits/types.h
rtk_main..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
rtk_main..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
rtk_main..o: /usr/include/endian.h /usr/include/features.h
rtk_main..o: /usr/include/_G_config.h /usr/include/gconv.h
rtk_main..o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/math.h
rtk_main..o: /usr/include/pthread.h /usr/include/sched.h /usr/include/signal.h
rtk_main..o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
rtk_main..o: /usr/include/string.h /usr/include/sys/cdefs.h
rtk_main..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
rtk_main..o: /usr/include/sys/types.h /usr/include/time.h /usr/include/wchar.h
rtk_main..o: /usr/include/xlocale.h
rtk_main..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
rtk_main..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
rtk_main..o: ../VERSION
server..o: ../include/entity.hh ../include/image.h ../include/ports.h
server..o: ../include/stage_types.hh ../include/world.hh /usr/include/alloca.h
server..o: /usr/include/arpa/inet.h /usr/include/asm/errno.h
server..o: /usr/include/asm/sigcontext.h /usr/include/asm/socket.h
server..o: /usr/include/asm/sockios.h /usr/include/assert.h
server..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
server..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
server..o: /usr/include/bits/errno.h /usr/include/bits/fcntl.h
server..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
server..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
server..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
server..o: /usr/include/bits/nan.h /usr/include/bits/netdb.h
server..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
server..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
server..o: /usr/include/bits/sched.h /usr/include/bits/select.h
server..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
server..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
server..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
server..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
server..o: /usr/include/bits/socket.h /usr/include/bits/stat.h
server..o: /usr/include/bits/stdio_lim.h /usr/include/bits/termios.h
server..o: /usr/include/bits/time.h /usr/include/bits/types.h
server..o: /usr/include/bits/uio.h /usr/include/bits/waitflags.h
server..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
server..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
server..o: /usr/include/endian.h /usr/include/errno.h /usr/include/fcntl.h
server..o: /usr/include/features.h /usr/include/g++-3/iomanip.h
server..o: /usr/include/g++-3/iostream.h /usr/include/g++-3/streambuf.h
server..o: /usr/include/g++-3/strfile.h /usr/include/g++-3/strstream.h
server..o: /usr/include/_G_config.h /usr/include/gconv.h /usr/include/getopt.h
server..o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/limits.h
server..o: /usr/include/linux/errno.h /usr/include/linux/limits.h
server..o: /usr/include/math.h /usr/include/netdb.h /usr/include/netinet/in.h
server..o: /usr/include/pthread.h /usr/include/rpc/netdb.h
server..o: /usr/include/sched.h /usr/include/signal.h /usr/include/stdint.h
server..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
server..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
server..o: /usr/include/sys/socket.h /usr/include/sys/stat.h
server..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
server..o: /usr/include/sys/ttydefaults.h /usr/include/sys/types.h
server..o: /usr/include/sys/ucontext.h /usr/include/sys/uio.h
server..o: /usr/include/termios.h /usr/include/time.h /usr/include/ucontext.h
server..o: /usr/include/unistd.h /usr/include/values.h /usr/include/wchar.h
server..o: /usr/include/xlocale.h
server..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/float.h
server..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
server..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
server..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
server..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
sonardevice..o: ../include/entity.hh ../include/image.h
sonardevice..o: ../include/playerdevice.hh ../include/sonardevice.hh
sonardevice..o: ../include/stage_types.hh ../include/world.hh
sonardevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
sonardevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
sonardevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
sonardevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
sonardevice..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
sonardevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
sonardevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
sonardevice..o: /usr/include/bits/posix2_lim.h
sonardevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
sonardevice..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
sonardevice..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
sonardevice..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
sonardevice..o: /usr/include/bits/time.h /usr/include/bits/types.h
sonardevice..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
sonardevice..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
sonardevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
sonardevice..o: /usr/include/features.h /usr/include/_G_config.h
sonardevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
sonardevice..o: /usr/include/libio.h /usr/include/limits.h
sonardevice..o: /usr/include/linux/limits.h /usr/include/math.h
sonardevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
sonardevice..o: /usr/include/sched.h /usr/include/signal.h
sonardevice..o: /usr/include/stdint.h /usr/include/stdio.h
sonardevice..o: /usr/include/stdlib.h /usr/include/string.h
sonardevice..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
sonardevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
sonardevice..o: /usr/include/time.h /usr/include/wchar.h
sonardevice..o: /usr/include/xlocale.h
sonardevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
sonardevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
sonardevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
sonardevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
sonardevice..o: /usr/local/player1.1.1beta/include/messages.h
sonardevice..o: /usr/local/player1.1.1beta/include/playercommon.h
sonardevice..o: /usr/local/player1.1.1beta/include/stage.h
usc_pioneer..o: ../include/entity.hh ../include/image.h
usc_pioneer..o: ../include/laserbeacondevice.hh ../include/laserdevice.hh
usc_pioneer..o: ../include/miscdevice.hh ../include/pioneermobiledevice.hh
usc_pioneer..o: ../include/playerdevice.hh ../include/playerserver.hh
usc_pioneer..o: ../include/ptzdevice.hh ../include/sonardevice.hh
usc_pioneer..o: ../include/stage_types.hh ../include/usc_pioneer.hh
usc_pioneer..o: ../include/visiondevice.hh ../include/world.hh
usc_pioneer..o: /usr/include/alloca.h /usr/include/asm/socket.h
usc_pioneer..o: /usr/include/asm/sockios.h /usr/include/assert.h
usc_pioneer..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
usc_pioneer..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
usc_pioneer..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
usc_pioneer..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
usc_pioneer..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
usc_pioneer..o: /usr/include/bits/posix2_lim.h
usc_pioneer..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
usc_pioneer..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
usc_pioneer..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
usc_pioneer..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
usc_pioneer..o: /usr/include/bits/time.h /usr/include/bits/types.h
usc_pioneer..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
usc_pioneer..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
usc_pioneer..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
usc_pioneer..o: /usr/include/features.h /usr/include/_G_config.h
usc_pioneer..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
usc_pioneer..o: /usr/include/libio.h /usr/include/limits.h
usc_pioneer..o: /usr/include/linux/limits.h /usr/include/math.h
usc_pioneer..o: /usr/include/netinet/in.h /usr/include/pthread.h
usc_pioneer..o: /usr/include/sched.h /usr/include/signal.h
usc_pioneer..o: /usr/include/stdint.h /usr/include/stdio.h
usc_pioneer..o: /usr/include/stdlib.h /usr/include/string.h
usc_pioneer..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
usc_pioneer..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
usc_pioneer..o: /usr/include/time.h /usr/include/wchar.h
usc_pioneer..o: /usr/include/xlocale.h
usc_pioneer..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
usc_pioneer..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
usc_pioneer..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
usc_pioneer..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
usc_pioneer..o: /usr/local/player1.1.1beta/include/messages.h
usc_pioneer..o: /usr/local/player1.1.1beta/include/playercommon.h
usc_pioneer..o: /usr/local/player1.1.1beta/include/stage.h
visionbeacon..o: ../include/entity.hh ../include/image.h
visionbeacon..o: ../include/stage_types.hh ../include/visionbeacon.hh
visionbeacon..o: ../include/world.hh /usr/include/alloca.h
visionbeacon..o: /usr/include/assert.h /usr/include/bits/endian.h
visionbeacon..o: /usr/include/bits/huge_val.h /usr/include/bits/initspin.h
visionbeacon..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
visionbeacon..o: /usr/include/bits/nan.h /usr/include/bits/pthreadtypes.h
visionbeacon..o: /usr/include/bits/sched.h /usr/include/bits/select.h
visionbeacon..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
visionbeacon..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
visionbeacon..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
visionbeacon..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
visionbeacon..o: /usr/include/bits/wordsize.h /usr/include/endian.h
visionbeacon..o: /usr/include/features.h /usr/include/_G_config.h
visionbeacon..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
visionbeacon..o: /usr/include/libio.h /usr/include/math.h
visionbeacon..o: /usr/include/pthread.h /usr/include/sched.h
visionbeacon..o: /usr/include/signal.h /usr/include/stdint.h
visionbeacon..o: /usr/include/stdio.h /usr/include/stdlib.h
visionbeacon..o: /usr/include/string.h /usr/include/sys/cdefs.h
visionbeacon..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
visionbeacon..o: /usr/include/sys/types.h /usr/include/time.h
visionbeacon..o: /usr/include/wchar.h /usr/include/xlocale.h
visionbeacon..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
visionbeacon..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
visiondevice..o: ../include/entity.hh ../include/image.h
visiondevice..o: ../include/playerdevice.hh ../include/ptzdevice.hh
visiondevice..o: ../include/stage_types.hh ../include/visiondevice.hh
visiondevice..o: ../include/world.hh /usr/include/alloca.h
visiondevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
visiondevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
visiondevice..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
visiondevice..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
visiondevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
visiondevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
visiondevice..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
visiondevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
visiondevice..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
visiondevice..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
visiondevice..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
visiondevice..o: /usr/include/bits/time.h /usr/include/bits/types.h
visiondevice..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
visiondevice..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
visiondevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
visiondevice..o: /usr/include/features.h /usr/include/_G_config.h
visiondevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
visiondevice..o: /usr/include/libio.h /usr/include/limits.h
visiondevice..o: /usr/include/linux/limits.h /usr/include/math.h
visiondevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
visiondevice..o: /usr/include/sched.h /usr/include/signal.h
visiondevice..o: /usr/include/stdint.h /usr/include/stdio.h
visiondevice..o: /usr/include/stdlib.h /usr/include/string.h
visiondevice..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
visiondevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
visiondevice..o: /usr/include/time.h /usr/include/wchar.h
visiondevice..o: /usr/include/xlocale.h
visiondevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
visiondevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
visiondevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
visiondevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
visiondevice..o: /usr/local/player1.1.1beta/include/messages.h
visiondevice..o: /usr/local/player1.1.1beta/include/playercommon.h
visiondevice..o: /usr/local/player1.1.1beta/include/stage.h
win..o: ../include/entity.hh ../include/image.h ../include/stage_types.hh
win..o: ../include/win.h ../include/world.hh /usr/include/alloca.h
win..o: /usr/include/asm/sigcontext.h /usr/include/assert.h
win..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
win..o: /usr/include/bits/environments.h /usr/include/bits/fcntl.h
win..o: /usr/include/bits/huge_val.h /usr/include/bits/initspin.h
win..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
win..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
win..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
win..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
win..o: /usr/include/bits/sched.h /usr/include/bits/select.h
win..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
win..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
win..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
win..o: /usr/include/bits/sigthread.h /usr/include/bits/stat.h
win..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
win..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
win..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
win..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
win..o: /usr/include/endian.h /usr/include/fcntl.h /usr/include/features.h
win..o: /usr/include/g++-3/iomanip.h /usr/include/g++-3/iostream.h
win..o: /usr/include/g++-3/streambuf.h /usr/include/g++-3/stream.h
win..o: /usr/include/_G_config.h /usr/include/gconv.h /usr/include/getopt.h
win..o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/limits.h
win..o: /usr/include/linux/limits.h /usr/include/math.h /usr/include/pthread.h
win..o: /usr/include/sched.h /usr/include/signal.h /usr/include/stdint.h
win..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
win..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
win..o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
win..o: /usr/include/sys/time.h /usr/include/sys/types.h
win..o: /usr/include/sys/ucontext.h /usr/include/time.h
win..o: /usr/include/ucontext.h /usr/include/unistd.h /usr/include/values.h
win..o: /usr/include/wchar.h /usr/include/X11/keysymdef.h
win..o: /usr/include/X11/keysym.h /usr/include/X11/Xarch.h
win..o: /usr/include/X11/Xfuncproto.h /usr/include/X11/X.h
win..o: /usr/include/X11/Xlib.h /usr/include/X11/Xosdefs.h
win..o: /usr/include/X11/Xos.h /usr/include/X11/Xutil.h /usr/include/xlocale.h
win..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/float.h
win..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
win..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
win..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
win..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
world_load..o: ../include/entityfactory.hh ../include/entity.hh
world_load..o: ../include/image.h ../include/stage_types.hh
world_load..o: ../include/world.hh /usr/include/alloca.h /usr/include/assert.h
world_load..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
world_load..o: /usr/include/bits/initspin.h /usr/include/bits/mathcalls.h
world_load..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
world_load..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
world_load..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
world_load..o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
world_load..o: /usr/include/bits/time.h /usr/include/bits/types.h
world_load..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
world_load..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
world_load..o: /usr/include/endian.h /usr/include/features.h
world_load..o: /usr/include/_G_config.h /usr/include/gconv.h
world_load..o: /usr/include/gnu/stubs.h /usr/include/libio.h
world_load..o: /usr/include/math.h /usr/include/pthread.h /usr/include/sched.h
world_load..o: /usr/include/signal.h /usr/include/stdint.h
world_load..o: /usr/include/stdio.h /usr/include/stdlib.h
world_load..o: /usr/include/string.h /usr/include/sys/cdefs.h
world_load..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
world_load..o: /usr/include/sys/types.h /usr/include/time.h
world_load..o: /usr/include/wchar.h /usr/include/xlocale.h
world_load..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
world_load..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
world..o: ../include/entity.hh ../include/image.h ../include/stage_types.hh
world..o: ../include/world.hh /usr/include/alloca.h
world..o: /usr/include/asm/sigcontext.h /usr/include/assert.h
world..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
world..o: /usr/include/bits/environments.h /usr/include/bits/huge_val.h
world..o: /usr/include/bits/initspin.h /usr/include/bits/mathcalls.h
world..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
world..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
world..o: /usr/include/bits/sched.h /usr/include/bits/select.h
world..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
world..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
world..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
world..o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
world..o: /usr/include/bits/time.h /usr/include/bits/types.h
world..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
world..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
world..o: /usr/include/endian.h /usr/include/features.h
world..o: /usr/include/_G_config.h /usr/include/gconv.h /usr/include/getopt.h
world..o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/math.h
world..o: /usr/include/pthread.h /usr/include/sched.h /usr/include/signal.h
world..o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
world..o: /usr/include/string.h /usr/include/sys/cdefs.h
world..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
world..o: /usr/include/sys/time.h /usr/include/sys/types.h
world..o: /usr/include/sys/ucontext.h /usr/include/time.h
world..o: /usr/include/ucontext.h /usr/include/unistd.h /usr/include/wchar.h
world..o: /usr/include/xlocale.h
world..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
world..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
world..o: /usr/local/player1.1.1beta/include/messages.h
world..o: /usr/local/player1.1.1beta/include/playercommon.h
world..o: /usr/local/player1.1.1beta/include/stage.h
xgui..o: ../include/entity.hh ../include/image.h ../include/stage_types.hh
xgui..o: ../include/world.hh ../include/xgui.hh /usr/include/alloca.h
xgui..o: /usr/include/asm/sigcontext.h /usr/include/assert.h
xgui..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
xgui..o: /usr/include/bits/environments.h /usr/include/bits/fcntl.h
xgui..o: /usr/include/bits/huge_val.h /usr/include/bits/initspin.h
xgui..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
xgui..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
xgui..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
xgui..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
xgui..o: /usr/include/bits/sched.h /usr/include/bits/select.h
xgui..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
xgui..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
xgui..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
xgui..o: /usr/include/bits/sigthread.h /usr/include/bits/stat.h
xgui..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
xgui..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
xgui..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
xgui..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
xgui..o: /usr/include/endian.h /usr/include/fcntl.h /usr/include/features.h
xgui..o: /usr/include/g++-3/fstream.h /usr/include/g++-3/iomanip.h
xgui..o: /usr/include/g++-3/iostream.h /usr/include/g++-3/streambuf.h
xgui..o: /usr/include/g++-3/stream.h /usr/include/_G_config.h
xgui..o: /usr/include/gconv.h /usr/include/getopt.h /usr/include/gnu/stubs.h
xgui..o: /usr/include/libio.h /usr/include/limits.h
xgui..o: /usr/include/linux/limits.h /usr/include/math.h
xgui..o: /usr/include/pthread.h /usr/include/sched.h /usr/include/signal.h
xgui..o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
xgui..o: /usr/include/string.h /usr/include/sys/cdefs.h
xgui..o: /usr/include/sys/select.h /usr/include/sys/stat.h
xgui..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
xgui..o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
xgui..o: /usr/include/time.h /usr/include/ucontext.h /usr/include/unistd.h
xgui..o: /usr/include/values.h /usr/include/wchar.h
xgui..o: /usr/include/X11/keysymdef.h /usr/include/X11/keysym.h
xgui..o: /usr/include/X11/Xarch.h /usr/include/X11/Xfuncproto.h
xgui..o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
xgui..o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xos.h
xgui..o: /usr/include/X11/Xutil.h /usr/include/xlocale.h
xgui..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/float.h
xgui..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
xgui..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
xgui..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
xgui..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
