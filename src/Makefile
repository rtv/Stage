
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


boxobstacle..o: ../include/boxobstacle.hh ../include/image.h
boxobstacle..o: ../include/object.hh ../include/playerrobot.hh
boxobstacle..o: ../include/stage_types.hh ../include/world.hh
boxobstacle..o: /usr/include/alloca.h /usr/include/assert.h
boxobstacle..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
boxobstacle..o: /usr/include/bits/initspin.h /usr/include/bits/mathcalls.h
boxobstacle..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
boxobstacle..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
boxobstacle..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
boxobstacle..o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
boxobstacle..o: /usr/include/bits/time.h /usr/include/bits/types.h
boxobstacle..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
boxobstacle..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
boxobstacle..o: /usr/include/endian.h /usr/include/features.h
boxobstacle..o: /usr/include/_G_config.h /usr/include/gconv.h
boxobstacle..o: /usr/include/gnu/stubs.h /usr/include/libio.h
boxobstacle..o: /usr/include/math.h /usr/include/pthread.h
boxobstacle..o: /usr/include/sched.h /usr/include/signal.h
boxobstacle..o: /usr/include/stdint.h /usr/include/stdio.h
boxobstacle..o: /usr/include/stdlib.h /usr/include/string.h
boxobstacle..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
boxobstacle..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
boxobstacle..o: /usr/include/time.h /usr/include/wchar.h
boxobstacle..o: /usr/include/xlocale.h
boxobstacle..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
boxobstacle..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
broadcastdevice..o: ../include/broadcastdevice.hh ../include/image.h
broadcastdevice..o: ../include/object.hh ../include/playerdevice.hh
broadcastdevice..o: ../include/playerrobot.hh ../include/stage_types.hh
broadcastdevice..o: ../include/world.hh /usr/include/alloca.h
broadcastdevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
broadcastdevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
broadcastdevice..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
broadcastdevice..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
broadcastdevice..o: /usr/include/bits/local_lim.h
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
laserbeacondevice..o: ../include/image.h ../include/laserbeacondevice.hh
laserbeacondevice..o: ../include/laserdevice.hh ../include/object.hh
laserbeacondevice..o: ../include/playerdevice.hh ../include/playerrobot.hh
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
laserbeacon..o: ../include/image.h ../include/laserbeacon.hh
laserbeacon..o: ../include/object.hh ../include/playerrobot.hh
laserbeacon..o: ../include/stage_types.hh ../include/world.hh
laserbeacon..o: /usr/include/alloca.h /usr/include/assert.h
laserbeacon..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
laserbeacon..o: /usr/include/bits/initspin.h /usr/include/bits/mathcalls.h
laserbeacon..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
laserbeacon..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
laserbeacon..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
laserbeacon..o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
laserbeacon..o: /usr/include/bits/time.h /usr/include/bits/types.h
laserbeacon..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
laserbeacon..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
laserbeacon..o: /usr/include/endian.h /usr/include/features.h
laserbeacon..o: /usr/include/_G_config.h /usr/include/gconv.h
laserbeacon..o: /usr/include/gnu/stubs.h /usr/include/libio.h
laserbeacon..o: /usr/include/math.h /usr/include/pthread.h
laserbeacon..o: /usr/include/sched.h /usr/include/signal.h
laserbeacon..o: /usr/include/stdint.h /usr/include/stdio.h
laserbeacon..o: /usr/include/stdlib.h /usr/include/string.h
laserbeacon..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
laserbeacon..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
laserbeacon..o: /usr/include/time.h /usr/include/wchar.h
laserbeacon..o: /usr/include/xlocale.h
laserbeacon..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
laserbeacon..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
laserdevice..o: ../include/image.h ../include/laserdevice.hh
laserdevice..o: ../include/object.hh ../include/playerdevice.hh
laserdevice..o: ../include/playerrobot.hh ../include/stage_types.hh
laserdevice..o: ../include/world.hh /usr/include/alloca.h
laserdevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
laserdevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
laserdevice..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
laserdevice..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
laserdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
laserdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
laserdevice..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
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
main..o: ../include/image.h ../include/object.hh ../include/playerrobot.hh
main..o: ../include/stage_types.hh ../include/world.hh /usr/include/alloca.h
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
miscdevice..o: ../include/image.h ../include/miscdevice.hh
miscdevice..o: ../include/object.hh ../include/playerdevice.hh
miscdevice..o: ../include/playerrobot.hh ../include/stage_types.hh
miscdevice..o: ../include/world.hh /usr/include/alloca.h
miscdevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
miscdevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
miscdevice..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
miscdevice..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
miscdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
miscdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
miscdevice..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
miscdevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
miscdevice..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
miscdevice..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
miscdevice..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
miscdevice..o: /usr/include/bits/time.h /usr/include/bits/types.h
miscdevice..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
miscdevice..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
miscdevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
miscdevice..o: /usr/include/features.h /usr/include/_G_config.h
miscdevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
miscdevice..o: /usr/include/libio.h /usr/include/limits.h
miscdevice..o: /usr/include/linux/limits.h /usr/include/math.h
miscdevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
miscdevice..o: /usr/include/sched.h /usr/include/signal.h
miscdevice..o: /usr/include/stdint.h /usr/include/stdio.h
miscdevice..o: /usr/include/stdlib.h /usr/include/string.h
miscdevice..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
miscdevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
miscdevice..o: /usr/include/time.h /usr/include/wchar.h /usr/include/xlocale.h
miscdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
miscdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
miscdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
miscdevice..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
miscdevice..o: /usr/local/player1.1.1beta/include/messages.h
miscdevice..o: /usr/local/player1.1.1beta/include/playercommon.h
miscdevice..o: /usr/local/player1.1.1beta/include/stage.h
objectfactory..o: ../include/boxobstacle.hh ../include/laserbeacon.hh
objectfactory..o: ../include/objectfactory.hh ../include/object.hh
objectfactory..o: ../include/stage_types.hh ../include/usc_pioneer.hh
objectfactory..o: ../include/visionbeacon.hh /usr/include/assert.h
objectfactory..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
objectfactory..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
objectfactory..o: /usr/include/bits/nan.h /usr/include/endian.h
objectfactory..o: /usr/include/features.h /usr/include/gnu/stubs.h
objectfactory..o: /usr/include/math.h /usr/include/string.h
objectfactory..o: /usr/include/sys/cdefs.h /usr/include/xlocale.h
objectfactory..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
object..o: ../include/object.hh ../include/stage_types.hh
object..o: /usr/include/alloca.h /usr/include/assert.h
object..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
object..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
object..o: /usr/include/bits/nan.h /usr/include/bits/pthreadtypes.h
object..o: /usr/include/bits/sched.h /usr/include/bits/select.h
object..o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
object..o: /usr/include/bits/time.h /usr/include/bits/types.h
object..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
object..o: /usr/include/bits/wchar.h /usr/include/endian.h
object..o: /usr/include/features.h /usr/include/_G_config.h
object..o: /usr/include/gconv.h /usr/include/gnu/stubs.h /usr/include/libio.h
object..o: /usr/include/math.h /usr/include/stdio.h /usr/include/stdlib.h
object..o: /usr/include/string.h /usr/include/sys/cdefs.h
object..o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
object..o: /usr/include/sys/types.h /usr/include/time.h /usr/include/wchar.h
object..o: /usr/include/xlocale.h
object..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
object..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
pioneermobiledevice..o: ../include/image.h ../include/object.hh
pioneermobiledevice..o: ../include/pioneermobiledevice.hh
pioneermobiledevice..o: ../include/playerdevice.hh ../include/playerrobot.hh
pioneermobiledevice..o: ../include/stage_types.hh ../include/world.hh
pioneermobiledevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
pioneermobiledevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
pioneermobiledevice..o: /usr/include/bits/byteswap.h
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
playerdevice..o: ../include/image.h ../include/object.hh
playerdevice..o: ../include/playerdevice.hh ../include/playerrobot.hh
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
playerrobot..o: ../include/image.h ../include/object.hh
playerrobot..o: ../include/playerrobot.hh ../include/stage_types.hh
playerrobot..o: ../include/world.hh /usr/include/alloca.h
playerrobot..o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
playerrobot..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
playerrobot..o: /usr/include/assert.h /usr/include/bits/byteswap.h
playerrobot..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
playerrobot..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
playerrobot..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
playerrobot..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
playerrobot..o: /usr/include/bits/ipc.h /usr/include/bits/local_lim.h
playerrobot..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
playerrobot..o: /usr/include/bits/mman.h /usr/include/bits/nan.h
playerrobot..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
playerrobot..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
playerrobot..o: /usr/include/bits/resource.h /usr/include/bits/sched.h
playerrobot..o: /usr/include/bits/select.h /usr/include/bits/sem.h
playerrobot..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
playerrobot..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
playerrobot..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
playerrobot..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
playerrobot..o: /usr/include/bits/socket.h /usr/include/bits/stat.h
playerrobot..o: /usr/include/bits/stdio_lim.h /usr/include/bits/termios.h
playerrobot..o: /usr/include/bits/time.h /usr/include/bits/types.h
playerrobot..o: /usr/include/bits/uio.h /usr/include/bits/waitflags.h
playerrobot..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
playerrobot..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
playerrobot..o: /usr/include/endian.h /usr/include/errno.h
playerrobot..o: /usr/include/fcntl.h /usr/include/features.h
playerrobot..o: /usr/include/g++-3/iomanip.h /usr/include/g++-3/iostream.h
playerrobot..o: /usr/include/g++-3/streambuf.h /usr/include/g++-3/strfile.h
playerrobot..o: /usr/include/g++-3/strstream.h /usr/include/_G_config.h
playerrobot..o: /usr/include/gconv.h /usr/include/getopt.h
playerrobot..o: /usr/include/gnu/stubs.h /usr/include/libio.h
playerrobot..o: /usr/include/limits.h /usr/include/linux/errno.h
playerrobot..o: /usr/include/linux/limits.h /usr/include/math.h
playerrobot..o: /usr/include/netinet/in.h /usr/include/pthread.h
playerrobot..o: /usr/include/sched.h /usr/include/signal.h
playerrobot..o: /usr/include/stdint.h /usr/include/stdio.h
playerrobot..o: /usr/include/stdlib.h /usr/include/string.h
playerrobot..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
playerrobot..o: /usr/include/sys/mman.h /usr/include/sys/resource.h
playerrobot..o: /usr/include/sys/select.h /usr/include/sys/sem.h
playerrobot..o: /usr/include/sys/socket.h /usr/include/sys/stat.h
playerrobot..o: /usr/include/sys/sysmacros.h /usr/include/sys/ttydefaults.h
playerrobot..o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
playerrobot..o: /usr/include/sys/uio.h /usr/include/sys/wait.h
playerrobot..o: /usr/include/termios.h /usr/include/time.h
playerrobot..o: /usr/include/ucontext.h /usr/include/unistd.h
playerrobot..o: /usr/include/wchar.h /usr/include/xlocale.h
playerrobot..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
playerrobot..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
playerrobot..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
playerrobot..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
playerrobot..o: /usr/local/player1.1.1beta/include/messages.h
playerrobot..o: /usr/local/player1.1.1beta/include/playercommon.h
playerrobot..o: /usr/local/player1.1.1beta/include/stage.h
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
ptzdevice..o: ../include/image.h ../include/object.hh
ptzdevice..o: ../include/playerdevice.hh ../include/playerrobot.hh
ptzdevice..o: ../include/ptzdevice.hh ../include/stage_types.hh
ptzdevice..o: ../include/world.hh /usr/include/alloca.h
ptzdevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
ptzdevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
ptzdevice..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
ptzdevice..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
ptzdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
ptzdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
ptzdevice..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
ptzdevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
ptzdevice..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
ptzdevice..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
ptzdevice..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
ptzdevice..o: /usr/include/bits/time.h /usr/include/bits/types.h
ptzdevice..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
ptzdevice..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
ptzdevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
ptzdevice..o: /usr/include/features.h /usr/include/_G_config.h
ptzdevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
ptzdevice..o: /usr/include/libio.h /usr/include/limits.h
ptzdevice..o: /usr/include/linux/limits.h /usr/include/math.h
ptzdevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
ptzdevice..o: /usr/include/sched.h /usr/include/signal.h /usr/include/stdint.h
ptzdevice..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
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
rtk_main..o: ../include/image.h ../include/object.hh ../include/playerrobot.hh
rtk_main..o: ../include/stage_types.hh ../include/world.hh
rtk_main..o: /usr/include/alloca.h /usr/include/assert.h
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
server..o: ../include/image.h ../include/object.hh ../include/playerrobot.hh
server..o: ../include/ports.h ../include/stage_types.hh ../include/world.hh
server..o: /usr/include/alloca.h /usr/include/arpa/inet.h
server..o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
server..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
server..o: /usr/include/assert.h /usr/include/bits/byteswap.h
server..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
server..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
server..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
server..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
server..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
server..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
server..o: /usr/include/bits/netdb.h /usr/include/bits/posix1_lim.h
server..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
server..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
server..o: /usr/include/bits/select.h /usr/include/bits/sigaction.h
server..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
server..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
server..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
server..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
server..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
server..o: /usr/include/bits/termios.h /usr/include/bits/time.h
server..o: /usr/include/bits/types.h /usr/include/bits/uio.h
server..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
server..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
server..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
server..o: /usr/include/errno.h /usr/include/fcntl.h /usr/include/features.h
server..o: /usr/include/g++-3/iomanip.h /usr/include/g++-3/iostream.h
server..o: /usr/include/g++-3/streambuf.h /usr/include/g++-3/strfile.h
server..o: /usr/include/g++-3/strstream.h /usr/include/_G_config.h
server..o: /usr/include/gconv.h /usr/include/getopt.h /usr/include/gnu/stubs.h
server..o: /usr/include/libio.h /usr/include/limits.h
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
sonardevice..o: ../include/image.h ../include/object.hh
sonardevice..o: ../include/playerdevice.hh ../include/playerrobot.hh
sonardevice..o: ../include/sonardevice.hh ../include/stage_types.hh
sonardevice..o: ../include/world.hh /usr/include/alloca.h
sonardevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
sonardevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
sonardevice..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
sonardevice..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
sonardevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
sonardevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
sonardevice..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
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
usc_pioneer..o: ../include/image.h ../include/laserbeacondevice.hh
usc_pioneer..o: ../include/laserdevice.hh ../include/miscdevice.hh
usc_pioneer..o: ../include/object.hh ../include/pioneermobiledevice.hh
usc_pioneer..o: ../include/playerdevice.hh ../include/playerrobot.hh
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
visionbeacon..o: ../include/image.h ../include/object.hh
visionbeacon..o: ../include/playerrobot.hh ../include/stage_types.hh
visionbeacon..o: ../include/visionbeacon.hh ../include/world.hh
visionbeacon..o: /usr/include/alloca.h /usr/include/assert.h
visionbeacon..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
visionbeacon..o: /usr/include/bits/initspin.h /usr/include/bits/mathcalls.h
visionbeacon..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
visionbeacon..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
visionbeacon..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
visionbeacon..o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
visionbeacon..o: /usr/include/bits/time.h /usr/include/bits/types.h
visionbeacon..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
visionbeacon..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
visionbeacon..o: /usr/include/endian.h /usr/include/features.h
visionbeacon..o: /usr/include/_G_config.h /usr/include/gconv.h
visionbeacon..o: /usr/include/gnu/stubs.h /usr/include/libio.h
visionbeacon..o: /usr/include/math.h /usr/include/pthread.h
visionbeacon..o: /usr/include/sched.h /usr/include/signal.h
visionbeacon..o: /usr/include/stdint.h /usr/include/stdio.h
visionbeacon..o: /usr/include/stdlib.h /usr/include/string.h
visionbeacon..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
visionbeacon..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
visionbeacon..o: /usr/include/time.h /usr/include/wchar.h
visionbeacon..o: /usr/include/xlocale.h
visionbeacon..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
visionbeacon..o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
visiondevice..o: ../include/image.h ../include/object.hh
visiondevice..o: ../include/playerdevice.hh ../include/playerrobot.hh
visiondevice..o: ../include/ptzdevice.hh ../include/stage_types.hh
visiondevice..o: ../include/visiondevice.hh ../include/world.hh
visiondevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
visiondevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
visiondevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
visiondevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
visiondevice..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
visiondevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
visiondevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
visiondevice..o: /usr/include/bits/posix2_lim.h
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
win..o: ../include/image.h ../include/object.hh ../include/playerrobot.hh
win..o: ../include/stage_types.hh ../include/win.h ../include/world.hh
win..o: /usr/include/alloca.h /usr/include/asm/sigcontext.h
win..o: /usr/include/assert.h /usr/include/bits/confname.h
win..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
win..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
win..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
win..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
win..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
win..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
win..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
win..o: /usr/include/bits/select.h /usr/include/bits/sigaction.h
win..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
win..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
win..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
win..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
win..o: /usr/include/bits/time.h /usr/include/bits/types.h
win..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
win..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
win..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
win..o: /usr/include/fcntl.h /usr/include/features.h
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
world_load..o: ../include/image.h ../include/objectfactory.hh
world_load..o: ../include/object.hh ../include/playerrobot.hh
world_load..o: ../include/stage_types.hh ../include/world.hh
world_load..o: /usr/include/alloca.h /usr/include/assert.h
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
world..o: ../include/image.h ../include/object.hh ../include/playerrobot.hh
world..o: ../include/stage_types.hh ../include/world.hh /usr/include/alloca.h
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
xgui..o: ../include/image.h ../include/object.hh ../include/playerrobot.hh
xgui..o: ../include/stage_types.hh ../include/world.hh ../include/xgui.hh
xgui..o: /usr/include/alloca.h /usr/include/asm/sigcontext.h
xgui..o: /usr/include/assert.h /usr/include/bits/confname.h
xgui..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
xgui..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
xgui..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
xgui..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
xgui..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
xgui..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
xgui..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
xgui..o: /usr/include/bits/select.h /usr/include/bits/sigaction.h
xgui..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
xgui..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
xgui..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
xgui..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
xgui..o: /usr/include/bits/time.h /usr/include/bits/types.h
xgui..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
xgui..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
xgui..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
xgui..o: /usr/include/fcntl.h /usr/include/features.h
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
