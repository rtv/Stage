
##############################################################################
# include the main configurations from the top-level directory
#

include ../../Makefile.common

##############################################################################
# Build section

# Objects common to all targets
#
OBJS =  world.o world_load.o truthserver.o envserver.o image.o \
	entity.o entityfactory.o playerdevice.o \
	positiondevice.o laserdevice.o sonardevice.o miscdevice.o \
	ptzdevice.o truthdevice.o occupancydevice.o boxobstacle.o \
	laserbeacon.o laserbeacondevice.o visiondevice.o visionbeacon.o \
	broadcastdevice.o puck.o gpsdevice.o gripperdevice.o main.o \
	matrix.o raytrace.o

# choose one of these that suits your build (debug/profile/production)

# production
#CCFLAGS = -O3 
# debug
#CCFLAGS = -g3 
# debug + profile
CCFLAGS = -pg -g3

# Flags common to all targets
#
CCFLAGS += -Wall -DREENTRANT -I../../include -I$(PLAYER_INC) $(ZLIB_FLAGS) 
CCFLAGS += -DVERSION=\"$(VERSION)\"

# production/debug link
#LINKFLAGS = -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
# profile link
LINKFLAGS = -pg -lm -lstdc++ -lpthread  $(ZLIB_LIBS)

#.SUFFIXES: .cc .o

##############################################################################
# Targets

all: stage

# Target stage with no GUI
#
stage : $(OBJS)
	$(LINKER) $(LINKFLAGS) -o $@ $(OBJS)  

clean:
	rm -f *.o *~ core stage

dep:
	$(MAKEDEP) -Y -- $(CCFLAGS) -- -s "# Dependancies (make dep)" ../*.cc 2>/dev/null
	$(RM) -f Makefile.bak

%.o : ../%.cc
	${CC} ${CCFLAGS} -c $< -o $@

##############################################################################
# Dependancies (make dep)


../boxobstacle..o: /home/vaughan/work/player/include/messages.h
../boxobstacle..o: /home/vaughan/work/player/include/playercommon.h
../boxobstacle..o: /home/vaughan/work/player/include/stage.h
../boxobstacle..o: /usr/include/alloca.h /usr/include/asm/socket.h
../boxobstacle..o: /usr/include/asm/sockios.h /usr/include/assert.h
../boxobstacle..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../boxobstacle..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../boxobstacle..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../boxobstacle..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../boxobstacle..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../boxobstacle..o: /usr/include/bits/posix1_lim.h
../boxobstacle..o: /usr/include/bits/posix2_lim.h
../boxobstacle..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../boxobstacle..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../boxobstacle..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../boxobstacle..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../boxobstacle..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../boxobstacle..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../boxobstacle..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../boxobstacle..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../boxobstacle..o: /usr/include/endian.h /usr/include/features.h
../boxobstacle..o: /usr/include/_G_config.h /usr/include/gconv.h
../boxobstacle..o: /usr/include/gnu/stubs.h /usr/include/libio.h
../boxobstacle..o: /usr/include/limits.h /usr/include/linux/limits.h
../boxobstacle..o: /usr/include/math.h /usr/include/netinet/in.h
../boxobstacle..o: /usr/include/pthread.h /usr/include/sched.h
../boxobstacle..o: /usr/include/signal.h /usr/include/stdint.h
../boxobstacle..o: /usr/include/stdio.h /usr/include/stdlib.h
../boxobstacle..o: /usr/include/string.h /usr/include/sys/cdefs.h
../boxobstacle..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../boxobstacle..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../boxobstacle..o: /usr/include/sys/time.h /usr/include/sys/types.h
../boxobstacle..o: /usr/include/time.h /usr/include/wchar.h
../boxobstacle..o: /usr/include/xlocale.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/concept_check.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/fpos.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/functexcept.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cassert.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_climits.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cstring.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_queue.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_construct.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_deque.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_function.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_heap.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_pair.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_queue.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_threads.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_vector.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stringfwd.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/type_traits.h
../boxobstacle..o: /usr/local/include/g++-v3/cstddef
../boxobstacle..o: /usr/local/include/g++-v3/cstdio
../boxobstacle..o: /usr/local/include/g++-v3/exception
../boxobstacle..o: /usr/local/include/g++-v3/exception_defines.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../boxobstacle..o: /usr/local/include/g++-v3/new
../boxobstacle..o: /usr/local/include/g++-v3/queue
../boxobstacle..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../boxobstacle..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../boxobstacle..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../boxobstacle..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../broadcastdevice..o: /home/vaughan/work/player/include/messages.h
../broadcastdevice..o: /home/vaughan/work/player/include/playercommon.h
../broadcastdevice..o: /home/vaughan/work/player/include/stage.h
../broadcastdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../broadcastdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../broadcastdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../broadcastdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../broadcastdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../broadcastdevice..o: /usr/include/bits/local_lim.h
../broadcastdevice..o: /usr/include/bits/mathcalls.h
../broadcastdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../broadcastdevice..o: /usr/include/bits/posix1_lim.h
../broadcastdevice..o: /usr/include/bits/posix2_lim.h
../broadcastdevice..o: /usr/include/bits/pthreadtypes.h
../broadcastdevice..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../broadcastdevice..o: /usr/include/bits/sem.h /usr/include/bits/sigset.h
../broadcastdevice..o: /usr/include/bits/sigthread.h
../broadcastdevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../broadcastdevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../broadcastdevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../broadcastdevice..o: /usr/include/bits/waitstatus.h
../broadcastdevice..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../broadcastdevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../broadcastdevice..o: /usr/include/features.h /usr/include/_G_config.h
../broadcastdevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
../broadcastdevice..o: /usr/include/libio.h /usr/include/limits.h
../broadcastdevice..o: /usr/include/linux/limits.h /usr/include/math.h
../broadcastdevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
../broadcastdevice..o: /usr/include/sched.h /usr/include/signal.h
../broadcastdevice..o: /usr/include/stdint.h /usr/include/stdio.h
../broadcastdevice..o: /usr/include/stdlib.h /usr/include/string.h
../broadcastdevice..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../broadcastdevice..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../broadcastdevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../broadcastdevice..o: /usr/include/sys/types.h /usr/include/time.h
../broadcastdevice..o: /usr/include/wchar.h /usr/include/xlocale.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../broadcastdevice..o: /usr/local/include/g++-v3/cstddef
../broadcastdevice..o: /usr/local/include/g++-v3/cstdio
../broadcastdevice..o: /usr/local/include/g++-v3/exception
../broadcastdevice..o: /usr/local/include/g++-v3/exception_defines.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../broadcastdevice..o: /usr/local/include/g++-v3/new
../broadcastdevice..o: /usr/local/include/g++-v3/queue
../broadcastdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../broadcastdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../broadcastdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../broadcastdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../entityfactory..o: /home/vaughan/work/player/include/messages.h
../entityfactory..o: /home/vaughan/work/player/include/playercommon.h
../entityfactory..o: /home/vaughan/work/player/include/stage.h
../entityfactory..o: /usr/include/alloca.h /usr/include/assert.h
../entityfactory..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
../entityfactory..o: /usr/include/bits/local_lim.h
../entityfactory..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../entityfactory..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../entityfactory..o: /usr/include/bits/posix2_lim.h
../entityfactory..o: /usr/include/bits/pthreadtypes.h
../entityfactory..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../entityfactory..o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
../entityfactory..o: /usr/include/bits/time.h /usr/include/bits/types.h
../entityfactory..o: /usr/include/bits/waitflags.h
../entityfactory..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../entityfactory..o: /usr/include/bits/wordsize.h
../entityfactory..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../entityfactory..o: /usr/include/features.h /usr/include/_G_config.h
../entityfactory..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
../entityfactory..o: /usr/include/libio.h /usr/include/limits.h
../entityfactory..o: /usr/include/linux/limits.h /usr/include/math.h
../entityfactory..o: /usr/include/stdint.h /usr/include/stdio.h
../entityfactory..o: /usr/include/stdlib.h /usr/include/string.h
../entityfactory..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
../entityfactory..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../entityfactory..o: /usr/include/sys/types.h /usr/include/time.h
../entityfactory..o: /usr/include/wchar.h /usr/include/xlocale.h
../entityfactory..o: /usr/local/include/g++-v3/backward/backward_warning.h
../entityfactory..o: /usr/local/include/g++-v3/backward/slist.h
../entityfactory..o: /usr/local/include/g++-v3/bits/concept_check.h
../entityfactory..o: /usr/local/include/g++-v3/bits/fpos.h
../entityfactory..o: /usr/local/include/g++-v3/bits/functexcept.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cassert.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_climits.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cstring.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_construct.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_pair.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_threads.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stringfwd.h
../entityfactory..o: /usr/local/include/g++-v3/bits/type_traits.h
../entityfactory..o: /usr/local/include/g++-v3/cstddef
../entityfactory..o: /usr/local/include/g++-v3/cstdio
../entityfactory..o: /usr/local/include/g++-v3/exception
../entityfactory..o: /usr/local/include/g++-v3/exception_defines.h
../entityfactory..o: /usr/local/include/g++-v3/ext/slist
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../entityfactory..o: /usr/local/include/g++-v3/new
../entityfactory..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../entityfactory..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../entityfactory..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../entityfactory..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../entity..o: /home/vaughan/work/player/include/messages.h
../entity..o: /home/vaughan/work/player/include/playercommon.h
../entity..o: /home/vaughan/work/player/include/stage.h /usr/include/alloca.h
../entity..o: /usr/include/asm/errno.h /usr/include/asm/socket.h
../entity..o: /usr/include/asm/sockios.h /usr/include/assert.h
../entity..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../entity..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../entity..o: /usr/include/bits/errno.h /usr/include/bits/huge_val.h
../entity..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
../entity..o: /usr/include/bits/ipc.h /usr/include/bits/locale.h
../entity..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../entity..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../entity..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../entity..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
../entity..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../entity..o: /usr/include/bits/sem.h /usr/include/bits/sigset.h
../entity..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
../entity..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
../entity..o: /usr/include/bits/time.h /usr/include/bits/types.h
../entity..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
../entity..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../entity..o: /usr/include/bits/xopen_lim.h /usr/include/ctype.h
../entity..o: /usr/include/endian.h /usr/include/errno.h
../entity..o: /usr/include/features.h /usr/include/_G_config.h
../entity..o: /usr/include/gconv.h /usr/include/getopt.h
../entity..o: /usr/include/gnu/stubs.h /usr/include/iconv.h
../entity..o: /usr/include/langinfo.h /usr/include/libio.h
../entity..o: /usr/include/limits.h /usr/include/linux/errno.h
../entity..o: /usr/include/linux/limits.h /usr/include/locale.h
../entity..o: /usr/include/math.h /usr/include/netinet/in.h
../entity..o: /usr/include/nl_types.h /usr/include/pthread.h
../entity..o: /usr/include/sched.h /usr/include/signal.h /usr/include/stdint.h
../entity..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
../entity..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../entity..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../entity..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../entity..o: /usr/include/sys/types.h /usr/include/time.h
../entity..o: /usr/include/unistd.h /usr/include/values.h /usr/include/wchar.h
../entity..o: /usr/include/wctype.h /usr/include/xlocale.h
../entity..o: /usr/local/include/g++-v3/backward/backward_warning.h
../entity..o: /usr/local/include/g++-v3/backward/iostream.h
../entity..o: /usr/local/include/g++-v3/bits/basic_ios.h
../entity..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../entity..o: /usr/local/include/g++-v3/bits/basic_string.h
../entity..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../entity..o: /usr/local/include/g++-v3/bits/char_traits.h
../entity..o: /usr/local/include/g++-v3/bits/codecvt.h
../entity..o: /usr/local/include/g++-v3/bits/concept_check.h
../entity..o: /usr/local/include/g++-v3/bits/fpos.h
../entity..o: /usr/local/include/g++-v3/bits/functexcept.h
../entity..o: /usr/local/include/g++-v3/bits/ios_base.h
../entity..o: /usr/local/include/g++-v3/bits/istream.tcc
../entity..o: /usr/local/include/g++-v3/bits/locale_facets.h
../entity..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../entity..o: /usr/local/include/g++-v3/bits/localefwd.h
../entity..o: /usr/local/include/g++-v3/bits/ostream.tcc
../entity..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../entity..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../entity..o: /usr/local/include/g++-v3/bits/std_cassert.h
../entity..o: /usr/local/include/g++-v3/bits/std_cctype.h
../entity..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../entity..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../entity..o: /usr/local/include/g++-v3/bits/std_climits.h
../entity..o: /usr/local/include/g++-v3/bits/std_clocale.h
../entity..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../entity..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../entity..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../entity..o: /usr/local/include/g++-v3/bits/std_cstring.h
../entity..o: /usr/local/include/g++-v3/bits/std_ctime.h
../entity..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../entity..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../entity..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../entity..o: /usr/local/include/g++-v3/bits/std_ios.h
../entity..o: /usr/local/include/g++-v3/bits/std_iostream.h
../entity..o: /usr/local/include/g++-v3/bits/std_istream.h
../entity..o: /usr/local/include/g++-v3/bits/std_iterator.h
../entity..o: /usr/local/include/g++-v3/bits/std_locale.h
../entity..o: /usr/local/include/g++-v3/bits/std_memory.h
../entity..o: /usr/local/include/g++-v3/bits/std_ostream.h
../entity..o: /usr/local/include/g++-v3/bits/std_queue.h
../entity..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../entity..o: /usr/local/include/g++-v3/bits/std_string.h
../entity..o: /usr/local/include/g++-v3/bits/std_vector.h
../entity..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../entity..o: /usr/local/include/g++-v3/bits/stl_algo.h
../entity..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../entity..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../entity..o: /usr/local/include/g++-v3/bits/stl_construct.h
../entity..o: /usr/local/include/g++-v3/bits/stl_deque.h
../entity..o: /usr/local/include/g++-v3/bits/stl_function.h
../entity..o: /usr/local/include/g++-v3/bits/stl_heap.h
../entity..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../entity..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../entity..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../entity..o: /usr/local/include/g++-v3/bits/stl_pair.h
../entity..o: /usr/local/include/g++-v3/bits/stl_queue.h
../entity..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../entity..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../entity..o: /usr/local/include/g++-v3/bits/stl_threads.h
../entity..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../entity..o: /usr/local/include/g++-v3/bits/stl_vector.h
../entity..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../entity..o: /usr/local/include/g++-v3/bits/stringfwd.h
../entity..o: /usr/local/include/g++-v3/bits/type_traits.h
../entity..o: /usr/local/include/g++-v3/cstddef
../entity..o: /usr/local/include/g++-v3/cstdio
../entity..o: /usr/local/include/g++-v3/exception
../entity..o: /usr/local/include/g++-v3/exception_defines.h
../entity..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../entity..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../entity..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../entity..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../entity..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../entity..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../entity..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../entity..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../entity..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../entity..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../entity..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../entity..o: /usr/local/include/g++-v3/typeinfo
../entity..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../entity..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../entity..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../entity..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../entity..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../envserver..o: /home/vaughan/work/player/include/messages.h
../envserver..o: /home/vaughan/work/player/include/playercommon.h
../envserver..o: /home/vaughan/work/player/include/stage.h
../envserver..o: /usr/include/alloca.h /usr/include/arpa/inet.h
../envserver..o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
../envserver..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../envserver..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../envserver..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../envserver..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
../envserver..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
../envserver..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
../envserver..o: /usr/include/bits/ipc.h /usr/include/bits/locale.h
../envserver..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../envserver..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../envserver..o: /usr/include/bits/netdb.h /usr/include/bits/posix1_lim.h
../envserver..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
../envserver..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../envserver..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../envserver..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
../envserver..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
../envserver..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
../envserver..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
../envserver..o: /usr/include/bits/socket.h /usr/include/bits/stat.h
../envserver..o: /usr/include/bits/stdio_lim.h /usr/include/bits/termios.h
../envserver..o: /usr/include/bits/time.h /usr/include/bits/types.h
../envserver..o: /usr/include/bits/uio.h /usr/include/bits/waitflags.h
../envserver..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../envserver..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../envserver..o: /usr/include/ctype.h /usr/include/endian.h
../envserver..o: /usr/include/errno.h /usr/include/fcntl.h
../envserver..o: /usr/include/features.h /usr/include/_G_config.h
../envserver..o: /usr/include/gconv.h /usr/include/getopt.h
../envserver..o: /usr/include/gnu/stubs.h /usr/include/iconv.h
../envserver..o: /usr/include/langinfo.h /usr/include/libio.h
../envserver..o: /usr/include/limits.h /usr/include/linux/errno.h
../envserver..o: /usr/include/linux/limits.h /usr/include/locale.h
../envserver..o: /usr/include/math.h /usr/include/netdb.h
../envserver..o: /usr/include/netinet/in.h /usr/include/nl_types.h
../envserver..o: /usr/include/pthread.h /usr/include/rpc/netdb.h
../envserver..o: /usr/include/sched.h /usr/include/signal.h
../envserver..o: /usr/include/stdint.h /usr/include/stdio.h
../envserver..o: /usr/include/stdlib.h /usr/include/string.h
../envserver..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../envserver..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../envserver..o: /usr/include/sys/socket.h /usr/include/sys/stat.h
../envserver..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../envserver..o: /usr/include/sys/ttydefaults.h /usr/include/sys/types.h
../envserver..o: /usr/include/sys/ucontext.h /usr/include/sys/uio.h
../envserver..o: /usr/include/termios.h /usr/include/time.h
../envserver..o: /usr/include/ucontext.h /usr/include/unistd.h
../envserver..o: /usr/include/wchar.h /usr/include/wctype.h
../envserver..o: /usr/include/xlocale.h
../envserver..o: /usr/local/include/g++-v3/backward/backward_warning.h
../envserver..o: /usr/local/include/g++-v3/backward/iomanip.h
../envserver..o: /usr/local/include/g++-v3/backward/iostream.h
../envserver..o: /usr/local/include/g++-v3/backward/strstream
../envserver..o: /usr/local/include/g++-v3/backward/strstream.h
../envserver..o: /usr/local/include/g++-v3/bits/basic_ios.h
../envserver..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../envserver..o: /usr/local/include/g++-v3/bits/basic_string.h
../envserver..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../envserver..o: /usr/local/include/g++-v3/bits/char_traits.h
../envserver..o: /usr/local/include/g++-v3/bits/codecvt.h
../envserver..o: /usr/local/include/g++-v3/bits/concept_check.h
../envserver..o: /usr/local/include/g++-v3/bits/fpos.h
../envserver..o: /usr/local/include/g++-v3/bits/functexcept.h
../envserver..o: /usr/local/include/g++-v3/bits/ios_base.h
../envserver..o: /usr/local/include/g++-v3/bits/istream.tcc
../envserver..o: /usr/local/include/g++-v3/bits/locale_facets.h
../envserver..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../envserver..o: /usr/local/include/g++-v3/bits/localefwd.h
../envserver..o: /usr/local/include/g++-v3/bits/ostream.tcc
../envserver..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../envserver..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../envserver..o: /usr/local/include/g++-v3/bits/std_cassert.h
../envserver..o: /usr/local/include/g++-v3/bits/std_cctype.h
../envserver..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../envserver..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../envserver..o: /usr/local/include/g++-v3/bits/std_climits.h
../envserver..o: /usr/local/include/g++-v3/bits/std_clocale.h
../envserver..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../envserver..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../envserver..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../envserver..o: /usr/local/include/g++-v3/bits/std_cstring.h
../envserver..o: /usr/local/include/g++-v3/bits/std_ctime.h
../envserver..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../envserver..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../envserver..o: /usr/local/include/g++-v3/bits/std_functional.h
../envserver..o: /usr/local/include/g++-v3/bits/std_iomanip.h
../envserver..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../envserver..o: /usr/local/include/g++-v3/bits/std_ios.h
../envserver..o: /usr/local/include/g++-v3/bits/std_iostream.h
../envserver..o: /usr/local/include/g++-v3/bits/std_istream.h
../envserver..o: /usr/local/include/g++-v3/bits/std_iterator.h
../envserver..o: /usr/local/include/g++-v3/bits/std_locale.h
../envserver..o: /usr/local/include/g++-v3/bits/std_memory.h
../envserver..o: /usr/local/include/g++-v3/bits/std_ostream.h
../envserver..o: /usr/local/include/g++-v3/bits/std_queue.h
../envserver..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../envserver..o: /usr/local/include/g++-v3/bits/std_string.h
../envserver..o: /usr/local/include/g++-v3/bits/std_vector.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_algo.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_construct.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_deque.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_function.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_heap.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_pair.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_queue.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_threads.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../envserver..o: /usr/local/include/g++-v3/bits/stl_vector.h
../envserver..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../envserver..o: /usr/local/include/g++-v3/bits/stringfwd.h
../envserver..o: /usr/local/include/g++-v3/bits/type_traits.h
../envserver..o: /usr/local/include/g++-v3/cstddef
../envserver..o: /usr/local/include/g++-v3/cstdio
../envserver..o: /usr/local/include/g++-v3/exception
../envserver..o: /usr/local/include/g++-v3/exception_defines.h
../envserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../envserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../envserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../envserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../envserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../envserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../envserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../envserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../envserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../envserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../envserver..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../envserver..o: /usr/local/include/g++-v3/typeinfo
../envserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../envserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../envserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../envserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../envserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../gpsdevice..o: /home/vaughan/work/player/include/messages.h
../gpsdevice..o: /home/vaughan/work/player/include/playercommon.h
../gpsdevice..o: /home/vaughan/work/player/include/stage.h
../gpsdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../gpsdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../gpsdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../gpsdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../gpsdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../gpsdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../gpsdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../gpsdevice..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../gpsdevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../gpsdevice..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../gpsdevice..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../gpsdevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../gpsdevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../gpsdevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../gpsdevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../gpsdevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../gpsdevice..o: /usr/include/endian.h /usr/include/features.h
../gpsdevice..o: /usr/include/_G_config.h /usr/include/gconv.h
../gpsdevice..o: /usr/include/gnu/stubs.h /usr/include/libio.h
../gpsdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../gpsdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../gpsdevice..o: /usr/include/pthread.h /usr/include/sched.h
../gpsdevice..o: /usr/include/signal.h /usr/include/stdint.h
../gpsdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../gpsdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../gpsdevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../gpsdevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../gpsdevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../gpsdevice..o: /usr/include/time.h /usr/include/wchar.h
../gpsdevice..o: /usr/include/xlocale.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../gpsdevice..o: /usr/local/include/g++-v3/cstddef
../gpsdevice..o: /usr/local/include/g++-v3/cstdio
../gpsdevice..o: /usr/local/include/g++-v3/exception
../gpsdevice..o: /usr/local/include/g++-v3/exception_defines.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../gpsdevice..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../gpsdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../gpsdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../gpsdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../gpsdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../gripperdevice..o: /home/vaughan/work/player/include/messages.h
../gripperdevice..o: /home/vaughan/work/player/include/playercommon.h
../gripperdevice..o: /home/vaughan/work/player/include/stage.h
../gripperdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../gripperdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../gripperdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../gripperdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../gripperdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../gripperdevice..o: /usr/include/bits/local_lim.h
../gripperdevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../gripperdevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../gripperdevice..o: /usr/include/bits/posix2_lim.h
../gripperdevice..o: /usr/include/bits/pthreadtypes.h
../gripperdevice..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../gripperdevice..o: /usr/include/bits/sem.h /usr/include/bits/sigset.h
../gripperdevice..o: /usr/include/bits/sigthread.h
../gripperdevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../gripperdevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../gripperdevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../gripperdevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../gripperdevice..o: /usr/include/bits/wordsize.h
../gripperdevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../gripperdevice..o: /usr/include/features.h /usr/include/_G_config.h
../gripperdevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
../gripperdevice..o: /usr/include/libio.h /usr/include/limits.h
../gripperdevice..o: /usr/include/linux/limits.h /usr/include/math.h
../gripperdevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
../gripperdevice..o: /usr/include/sched.h /usr/include/signal.h
../gripperdevice..o: /usr/include/stdint.h /usr/include/stdio.h
../gripperdevice..o: /usr/include/stdlib.h /usr/include/string.h
../gripperdevice..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../gripperdevice..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../gripperdevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../gripperdevice..o: /usr/include/sys/types.h /usr/include/time.h
../gripperdevice..o: /usr/include/values.h /usr/include/wchar.h
../gripperdevice..o: /usr/include/xlocale.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../gripperdevice..o: /usr/local/include/g++-v3/cstddef
../gripperdevice..o: /usr/local/include/g++-v3/cstdio
../gripperdevice..o: /usr/local/include/g++-v3/exception
../gripperdevice..o: /usr/local/include/g++-v3/exception_defines.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../gripperdevice..o: /usr/local/include/g++-v3/new
../gripperdevice..o: /usr/local/include/g++-v3/queue
../gripperdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../gripperdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../gripperdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../gripperdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../gripperdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../image..o: /usr/include/alloca.h /usr/include/asm/errno.h
../image..o: /usr/include/assert.h /usr/include/bits/endian.h
../image..o: /usr/include/bits/errno.h /usr/include/bits/huge_val.h
../image..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
../image..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../image..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../image..o: /usr/include/bits/posix2_lim.h /usr/include/bits/pthreadtypes.h
../image..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../image..o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
../image..o: /usr/include/bits/time.h /usr/include/bits/types.h
../image..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
../image..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../image..o: /usr/include/bits/xopen_lim.h /usr/include/ctype.h
../image..o: /usr/include/endian.h /usr/include/errno.h
../image..o: /usr/include/features.h /usr/include/_G_config.h
../image..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
../image..o: /usr/include/iconv.h /usr/include/langinfo.h /usr/include/libio.h
../image..o: /usr/include/limits.h /usr/include/linux/errno.h
../image..o: /usr/include/linux/limits.h /usr/include/locale.h
../image..o: /usr/include/math.h /usr/include/nl_types.h /usr/include/stdio.h
../image..o: /usr/include/stdlib.h /usr/include/string.h
../image..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
../image..o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
../image..o: /usr/include/time.h /usr/include/wchar.h /usr/include/wctype.h
../image..o: /usr/include/xlocale.h /usr/include/zconf.h /usr/include/zlib.h
../image..o: /usr/local/include/g++-v3/backward/backward_warning.h
../image..o: /usr/local/include/g++-v3/backward/fstream.h
../image..o: /usr/local/include/g++-v3/backward/iostream.h
../image..o: /usr/local/include/g++-v3/bits/basic_file.h
../image..o: /usr/local/include/g++-v3/bits/basic_ios.h
../image..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../image..o: /usr/local/include/g++-v3/bits/basic_string.h
../image..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../image..o: /usr/local/include/g++-v3/bits/char_traits.h
../image..o: /usr/local/include/g++-v3/bits/codecvt.h
../image..o: /usr/local/include/g++-v3/bits/concept_check.h
../image..o: /usr/local/include/g++-v3/bits/fpos.h
../image..o: /usr/local/include/g++-v3/bits/fstream.tcc
../image..o: /usr/local/include/g++-v3/bits/functexcept.h
../image..o: /usr/local/include/g++-v3/bits/ios_base.h
../image..o: /usr/local/include/g++-v3/bits/istream.tcc
../image..o: /usr/local/include/g++-v3/bits/locale_facets.h
../image..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../image..o: /usr/local/include/g++-v3/bits/localefwd.h
../image..o: /usr/local/include/g++-v3/bits/ostream.tcc
../image..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../image..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../image..o: /usr/local/include/g++-v3/bits/std_cassert.h
../image..o: /usr/local/include/g++-v3/bits/std_cctype.h
../image..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../image..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../image..o: /usr/local/include/g++-v3/bits/std_climits.h
../image..o: /usr/local/include/g++-v3/bits/std_clocale.h
../image..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../image..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../image..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../image..o: /usr/local/include/g++-v3/bits/std_cstring.h
../image..o: /usr/local/include/g++-v3/bits/std_ctime.h
../image..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../image..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../image..o: /usr/local/include/g++-v3/bits/std_fstream.h
../image..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../image..o: /usr/local/include/g++-v3/bits/std_ios.h
../image..o: /usr/local/include/g++-v3/bits/std_iostream.h
../image..o: /usr/local/include/g++-v3/bits/std_istream.h
../image..o: /usr/local/include/g++-v3/bits/std_iterator.h
../image..o: /usr/local/include/g++-v3/bits/std_locale.h
../image..o: /usr/local/include/g++-v3/bits/std_memory.h
../image..o: /usr/local/include/g++-v3/bits/std_ostream.h
../image..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../image..o: /usr/local/include/g++-v3/bits/std_string.h
../image..o: /usr/local/include/g++-v3/bits/std_vector.h
../image..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../image..o: /usr/local/include/g++-v3/bits/stl_algo.h
../image..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../image..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../image..o: /usr/local/include/g++-v3/bits/stl_construct.h
../image..o: /usr/local/include/g++-v3/bits/stl_heap.h
../image..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../image..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../image..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../image..o: /usr/local/include/g++-v3/bits/stl_pair.h
../image..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../image..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../image..o: /usr/local/include/g++-v3/bits/stl_threads.h
../image..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../image..o: /usr/local/include/g++-v3/bits/stl_vector.h
../image..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../image..o: /usr/local/include/g++-v3/bits/stringfwd.h
../image..o: /usr/local/include/g++-v3/bits/type_traits.h
../image..o: /usr/local/include/g++-v3/cstddef
../image..o: /usr/local/include/g++-v3/cstdio
../image..o: /usr/local/include/g++-v3/exception
../image..o: /usr/local/include/g++-v3/exception_defines.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/basic_file_model.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../image..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../image..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/typeinfo
../image..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../image..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../image..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../image..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../image..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../laserbeacondevice..o: /home/vaughan/work/player/include/messages.h
../laserbeacondevice..o: /home/vaughan/work/player/include/playercommon.h
../laserbeacondevice..o: /home/vaughan/work/player/include/stage.h
../laserbeacondevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../laserbeacondevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../laserbeacondevice..o: /usr/include/bits/byteswap.h
../laserbeacondevice..o: /usr/include/bits/endian.h
../laserbeacondevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../laserbeacondevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../laserbeacondevice..o: /usr/include/bits/local_lim.h
../laserbeacondevice..o: /usr/include/bits/mathcalls.h
../laserbeacondevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../laserbeacondevice..o: /usr/include/bits/posix1_lim.h
../laserbeacondevice..o: /usr/include/bits/posix2_lim.h
../laserbeacondevice..o: /usr/include/bits/pthreadtypes.h
../laserbeacondevice..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../laserbeacondevice..o: /usr/include/bits/sem.h /usr/include/bits/sigset.h
../laserbeacondevice..o: /usr/include/bits/sigthread.h
../laserbeacondevice..o: /usr/include/bits/sockaddr.h
../laserbeacondevice..o: /usr/include/bits/socket.h
../laserbeacondevice..o: /usr/include/bits/stdio_lim.h
../laserbeacondevice..o: /usr/include/bits/time.h /usr/include/bits/types.h
../laserbeacondevice..o: /usr/include/bits/waitflags.h
../laserbeacondevice..o: /usr/include/bits/waitstatus.h
../laserbeacondevice..o: /usr/include/bits/wchar.h
../laserbeacondevice..o: /usr/include/bits/wordsize.h
../laserbeacondevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../laserbeacondevice..o: /usr/include/features.h /usr/include/_G_config.h
../laserbeacondevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
../laserbeacondevice..o: /usr/include/libio.h /usr/include/limits.h
../laserbeacondevice..o: /usr/include/linux/limits.h /usr/include/math.h
../laserbeacondevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
../laserbeacondevice..o: /usr/include/sched.h /usr/include/signal.h
../laserbeacondevice..o: /usr/include/stdint.h /usr/include/stdio.h
../laserbeacondevice..o: /usr/include/stdlib.h /usr/include/string.h
../laserbeacondevice..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../laserbeacondevice..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../laserbeacondevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../laserbeacondevice..o: /usr/include/sys/types.h /usr/include/time.h
../laserbeacondevice..o: /usr/include/wchar.h /usr/include/xlocale.h
../laserbeacondevice..o: /usr/local/include/g++-v3/backward/backward_warning.h
../laserbeacondevice..o: /usr/local/include/g++-v3/backward/slist.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/fpos.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../laserbeacondevice..o: /usr/local/include/g++-v3/cstddef
../laserbeacondevice..o: /usr/local/include/g++-v3/cstdio
../laserbeacondevice..o: /usr/local/include/g++-v3/exception
../laserbeacondevice..o: /usr/local/include/g++-v3/exception_defines.h
../laserbeacondevice..o: /usr/local/include/g++-v3/ext/slist
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../laserbeacondevice..o: /usr/local/include/g++-v3/new
../laserbeacondevice..o: /usr/local/include/g++-v3/queue
../laserbeacondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../laserbeacondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../laserbeacondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../laserbeacondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../laserbeacon..o: /home/vaughan/work/player/include/messages.h
../laserbeacon..o: /home/vaughan/work/player/include/playercommon.h
../laserbeacon..o: /home/vaughan/work/player/include/stage.h
../laserbeacon..o: /usr/include/alloca.h /usr/include/asm/socket.h
../laserbeacon..o: /usr/include/asm/sockios.h /usr/include/assert.h
../laserbeacon..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../laserbeacon..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../laserbeacon..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../laserbeacon..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../laserbeacon..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../laserbeacon..o: /usr/include/bits/posix1_lim.h
../laserbeacon..o: /usr/include/bits/posix2_lim.h
../laserbeacon..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../laserbeacon..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../laserbeacon..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../laserbeacon..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../laserbeacon..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../laserbeacon..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../laserbeacon..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../laserbeacon..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../laserbeacon..o: /usr/include/endian.h /usr/include/features.h
../laserbeacon..o: /usr/include/_G_config.h /usr/include/gconv.h
../laserbeacon..o: /usr/include/gnu/stubs.h /usr/include/libio.h
../laserbeacon..o: /usr/include/limits.h /usr/include/linux/limits.h
../laserbeacon..o: /usr/include/math.h /usr/include/netinet/in.h
../laserbeacon..o: /usr/include/pthread.h /usr/include/sched.h
../laserbeacon..o: /usr/include/signal.h /usr/include/stdint.h
../laserbeacon..o: /usr/include/stdio.h /usr/include/stdlib.h
../laserbeacon..o: /usr/include/string.h /usr/include/sys/cdefs.h
../laserbeacon..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../laserbeacon..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../laserbeacon..o: /usr/include/sys/time.h /usr/include/sys/types.h
../laserbeacon..o: /usr/include/time.h /usr/include/wchar.h
../laserbeacon..o: /usr/include/xlocale.h
../laserbeacon..o: /usr/local/include/g++-v3/backward/backward_warning.h
../laserbeacon..o: /usr/local/include/g++-v3/backward/slist.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/concept_check.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/fpos.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/functexcept.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cassert.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_climits.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cstring.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_queue.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_construct.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_deque.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_function.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_heap.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_pair.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_queue.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_threads.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_vector.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stringfwd.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/type_traits.h
../laserbeacon..o: /usr/local/include/g++-v3/cstddef
../laserbeacon..o: /usr/local/include/g++-v3/cstdio
../laserbeacon..o: /usr/local/include/g++-v3/exception
../laserbeacon..o: /usr/local/include/g++-v3/exception_defines.h
../laserbeacon..o: /usr/local/include/g++-v3/ext/slist
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../laserbeacon..o: /usr/local/include/g++-v3/new
../laserbeacon..o: /usr/local/include/g++-v3/queue
../laserbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../laserbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../laserbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../laserbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../laserdevice..o: /home/vaughan/work/player/include/messages.h
../laserdevice..o: /home/vaughan/work/player/include/playercommon.h
../laserdevice..o: /home/vaughan/work/player/include/stage.h
../laserdevice..o: /usr/include/alloca.h /usr/include/asm/errno.h
../laserdevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../laserdevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../laserdevice..o: /usr/include/bits/endian.h /usr/include/bits/errno.h
../laserdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../laserdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../laserdevice..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
../laserdevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../laserdevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../laserdevice..o: /usr/include/bits/posix2_lim.h
../laserdevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../laserdevice..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../laserdevice..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../laserdevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../laserdevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../laserdevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../laserdevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../laserdevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../laserdevice..o: /usr/include/ctype.h /usr/include/endian.h
../laserdevice..o: /usr/include/errno.h /usr/include/features.h
../laserdevice..o: /usr/include/_G_config.h /usr/include/gconv.h
../laserdevice..o: /usr/include/gnu/stubs.h /usr/include/iconv.h
../laserdevice..o: /usr/include/langinfo.h /usr/include/libio.h
../laserdevice..o: /usr/include/limits.h /usr/include/linux/errno.h
../laserdevice..o: /usr/include/linux/limits.h /usr/include/locale.h
../laserdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../laserdevice..o: /usr/include/nl_types.h /usr/include/pthread.h
../laserdevice..o: /usr/include/sched.h /usr/include/signal.h
../laserdevice..o: /usr/include/stdint.h /usr/include/stdio.h
../laserdevice..o: /usr/include/stdlib.h /usr/include/string.h
../laserdevice..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../laserdevice..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../laserdevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../laserdevice..o: /usr/include/sys/types.h /usr/include/time.h
../laserdevice..o: /usr/include/wchar.h /usr/include/wctype.h
../laserdevice..o: /usr/include/xlocale.h
../laserdevice..o: /usr/local/include/g++-v3/backward/backward_warning.h
../laserdevice..o: /usr/local/include/g++-v3/backward/iostream.h
../laserdevice..o: /usr/local/include/g++-v3/backward/slist.h
../laserdevice..o: /usr/local/include/g++-v3/bits/basic_ios.h
../laserdevice..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../laserdevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../laserdevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../laserdevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../laserdevice..o: /usr/local/include/g++-v3/bits/codecvt.h
../laserdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../laserdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../laserdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../laserdevice..o: /usr/local/include/g++-v3/bits/ios_base.h
../laserdevice..o: /usr/local/include/g++-v3/bits/istream.tcc
../laserdevice..o: /usr/local/include/g++-v3/bits/locale_facets.h
../laserdevice..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../laserdevice..o: /usr/local/include/g++-v3/bits/localefwd.h
../laserdevice..o: /usr/local/include/g++-v3/bits/ostream.tcc
../laserdevice..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_cctype.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_clocale.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_ctime.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_ios.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_iostream.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_istream.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_locale.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_ostream.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_string.h
../laserdevice..o: /usr/local/include/g++-v3/bits/std_vector.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../laserdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../laserdevice..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../laserdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../laserdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../laserdevice..o: /usr/local/include/g++-v3/cstddef
../laserdevice..o: /usr/local/include/g++-v3/cstdio
../laserdevice..o: /usr/local/include/g++-v3/exception
../laserdevice..o: /usr/local/include/g++-v3/exception_defines.h
../laserdevice..o: /usr/local/include/g++-v3/ext/slist
../laserdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../laserdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../laserdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../laserdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../laserdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../laserdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../laserdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../laserdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../laserdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../laserdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../laserdevice..o: /usr/local/include/g++-v3/new
../laserdevice..o: /usr/local/include/g++-v3/queue
../laserdevice..o: /usr/local/include/g++-v3/typeinfo
../laserdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../laserdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../laserdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../laserdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../laserdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../main..o: /home/vaughan/work/player/include/messages.h
../main..o: /home/vaughan/work/player/include/playercommon.h
../main..o: /home/vaughan/work/player/include/stage.h /usr/include/alloca.h
../main..o: /usr/include/asm/sigcontext.h /usr/include/asm/socket.h
../main..o: /usr/include/asm/sockios.h /usr/include/assert.h
../main..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../main..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../main..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../main..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../main..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../main..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../main..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../main..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
../main..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../main..o: /usr/include/bits/sem.h /usr/include/bits/sigaction.h
../main..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
../main..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
../main..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
../main..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../main..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../main..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../main..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../main..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../main..o: /usr/include/endian.h /usr/include/features.h
../main..o: /usr/include/_G_config.h /usr/include/gconv.h
../main..o: /usr/include/getopt.h /usr/include/gnu/stubs.h
../main..o: /usr/include/libio.h /usr/include/limits.h
../main..o: /usr/include/linux/limits.h /usr/include/math.h
../main..o: /usr/include/netinet/in.h /usr/include/pthread.h
../main..o: /usr/include/sched.h /usr/include/signal.h /usr/include/stdint.h
../main..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
../main..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../main..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../main..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../main..o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
../main..o: /usr/include/time.h /usr/include/ucontext.h /usr/include/unistd.h
../main..o: /usr/include/wchar.h /usr/include/xlocale.h
../main..o: /usr/local/include/g++-v3/bits/concept_check.h
../main..o: /usr/local/include/g++-v3/bits/fpos.h
../main..o: /usr/local/include/g++-v3/bits/functexcept.h
../main..o: /usr/local/include/g++-v3/bits/std_cassert.h
../main..o: /usr/local/include/g++-v3/bits/std_climits.h
../main..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../main..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../main..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../main..o: /usr/local/include/g++-v3/bits/std_cstring.h
../main..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../main..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../main..o: /usr/local/include/g++-v3/bits/std_queue.h
../main..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../main..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../main..o: /usr/local/include/g++-v3/bits/stl_construct.h
../main..o: /usr/local/include/g++-v3/bits/stl_deque.h
../main..o: /usr/local/include/g++-v3/bits/stl_function.h
../main..o: /usr/local/include/g++-v3/bits/stl_heap.h
../main..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../main..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../main..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../main..o: /usr/local/include/g++-v3/bits/stl_pair.h
../main..o: /usr/local/include/g++-v3/bits/stl_queue.h
../main..o: /usr/local/include/g++-v3/bits/stl_threads.h
../main..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../main..o: /usr/local/include/g++-v3/bits/stl_vector.h
../main..o: /usr/local/include/g++-v3/bits/stringfwd.h
../main..o: /usr/local/include/g++-v3/bits/type_traits.h
../main..o: /usr/local/include/g++-v3/cstddef /usr/local/include/g++-v3/cstdio
../main..o: /usr/local/include/g++-v3/exception
../main..o: /usr/local/include/g++-v3/exception_defines.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../main..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../matrix..o: /home/vaughan/work/player/include/messages.h
../matrix..o: /home/vaughan/work/player/include/playercommon.h
../matrix..o: /home/vaughan/work/player/include/stage.h /usr/include/alloca.h
../matrix..o: /usr/include/asm/errno.h /usr/include/assert.h
../matrix..o: /usr/include/bits/endian.h /usr/include/bits/errno.h
../matrix..o: /usr/include/bits/huge_val.h /usr/include/bits/locale.h
../matrix..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../matrix..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../matrix..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../matrix..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../matrix..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
../matrix..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../matrix..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../matrix..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../matrix..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../matrix..o: /usr/include/ctype.h /usr/include/endian.h /usr/include/errno.h
../matrix..o: /usr/include/features.h /usr/include/_G_config.h
../matrix..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
../matrix..o: /usr/include/iconv.h /usr/include/langinfo.h
../matrix..o: /usr/include/libio.h /usr/include/limits.h
../matrix..o: /usr/include/linux/errno.h /usr/include/linux/limits.h
../matrix..o: /usr/include/locale.h /usr/include/math.h
../matrix..o: /usr/include/nl_types.h /usr/include/stdint.h
../matrix..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
../matrix..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
../matrix..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../matrix..o: /usr/include/sys/types.h /usr/include/time.h
../matrix..o: /usr/include/wchar.h /usr/include/wctype.h
../matrix..o: /usr/include/xlocale.h /usr/include/zconf.h /usr/include/zlib.h
../matrix..o: /usr/local/include/g++-v3/backward/backward_warning.h
../matrix..o: /usr/local/include/g++-v3/backward/fstream.h
../matrix..o: /usr/local/include/g++-v3/backward/iostream.h
../matrix..o: /usr/local/include/g++-v3/bits/basic_file.h
../matrix..o: /usr/local/include/g++-v3/bits/basic_ios.h
../matrix..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../matrix..o: /usr/local/include/g++-v3/bits/basic_string.h
../matrix..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../matrix..o: /usr/local/include/g++-v3/bits/char_traits.h
../matrix..o: /usr/local/include/g++-v3/bits/codecvt.h
../matrix..o: /usr/local/include/g++-v3/bits/concept_check.h
../matrix..o: /usr/local/include/g++-v3/bits/fpos.h
../matrix..o: /usr/local/include/g++-v3/bits/fstream.tcc
../matrix..o: /usr/local/include/g++-v3/bits/functexcept.h
../matrix..o: /usr/local/include/g++-v3/bits/ios_base.h
../matrix..o: /usr/local/include/g++-v3/bits/istream.tcc
../matrix..o: /usr/local/include/g++-v3/bits/locale_facets.h
../matrix..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../matrix..o: /usr/local/include/g++-v3/bits/localefwd.h
../matrix..o: /usr/local/include/g++-v3/bits/ostream.tcc
../matrix..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../matrix..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../matrix..o: /usr/local/include/g++-v3/bits/std_cassert.h
../matrix..o: /usr/local/include/g++-v3/bits/std_cctype.h
../matrix..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../matrix..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../matrix..o: /usr/local/include/g++-v3/bits/std_climits.h
../matrix..o: /usr/local/include/g++-v3/bits/std_clocale.h
../matrix..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../matrix..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../matrix..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../matrix..o: /usr/local/include/g++-v3/bits/std_cstring.h
../matrix..o: /usr/local/include/g++-v3/bits/std_ctime.h
../matrix..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../matrix..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../matrix..o: /usr/local/include/g++-v3/bits/std_fstream.h
../matrix..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../matrix..o: /usr/local/include/g++-v3/bits/std_ios.h
../matrix..o: /usr/local/include/g++-v3/bits/std_iostream.h
../matrix..o: /usr/local/include/g++-v3/bits/std_istream.h
../matrix..o: /usr/local/include/g++-v3/bits/std_iterator.h
../matrix..o: /usr/local/include/g++-v3/bits/std_locale.h
../matrix..o: /usr/local/include/g++-v3/bits/std_memory.h
../matrix..o: /usr/local/include/g++-v3/bits/std_ostream.h
../matrix..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../matrix..o: /usr/local/include/g++-v3/bits/std_string.h
../matrix..o: /usr/local/include/g++-v3/bits/std_vector.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_algo.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_construct.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_heap.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_pair.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_threads.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../matrix..o: /usr/local/include/g++-v3/bits/stl_vector.h
../matrix..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../matrix..o: /usr/local/include/g++-v3/bits/stringfwd.h
../matrix..o: /usr/local/include/g++-v3/bits/type_traits.h
../matrix..o: /usr/local/include/g++-v3/cstddef
../matrix..o: /usr/local/include/g++-v3/cstdio
../matrix..o: /usr/local/include/g++-v3/exception
../matrix..o: /usr/local/include/g++-v3/exception_defines.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/basic_file_model.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../matrix..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../matrix..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/typeinfo
../matrix..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../matrix..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../matrix..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../matrix..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../matrix..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../miscdevice..o: /home/vaughan/work/player/include/messages.h
../miscdevice..o: /home/vaughan/work/player/include/playercommon.h
../miscdevice..o: /home/vaughan/work/player/include/stage.h
../miscdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../miscdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../miscdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../miscdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../miscdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../miscdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../miscdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../miscdevice..o: /usr/include/bits/posix1_lim.h
../miscdevice..o: /usr/include/bits/posix2_lim.h
../miscdevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../miscdevice..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../miscdevice..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../miscdevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../miscdevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../miscdevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../miscdevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../miscdevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../miscdevice..o: /usr/include/endian.h /usr/include/features.h
../miscdevice..o: /usr/include/_G_config.h /usr/include/gconv.h
../miscdevice..o: /usr/include/gnu/stubs.h /usr/include/libio.h
../miscdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../miscdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../miscdevice..o: /usr/include/pthread.h /usr/include/sched.h
../miscdevice..o: /usr/include/signal.h /usr/include/stdint.h
../miscdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../miscdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../miscdevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../miscdevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../miscdevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../miscdevice..o: /usr/include/time.h /usr/include/wchar.h
../miscdevice..o: /usr/include/xlocale.h
../miscdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../miscdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../miscdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../miscdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../miscdevice..o: /usr/local/include/g++-v3/cstddef
../miscdevice..o: /usr/local/include/g++-v3/cstdio
../miscdevice..o: /usr/local/include/g++-v3/exception
../miscdevice..o: /usr/local/include/g++-v3/exception_defines.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../miscdevice..o: /usr/local/include/g++-v3/new
../miscdevice..o: /usr/local/include/g++-v3/queue
../miscdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../miscdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../miscdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../miscdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../occupancydevice..o: /home/vaughan/work/player/include/messages.h
../occupancydevice..o: /home/vaughan/work/player/include/playercommon.h
../occupancydevice..o: /home/vaughan/work/player/include/stage.h
../occupancydevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../occupancydevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../occupancydevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../occupancydevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../occupancydevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../occupancydevice..o: /usr/include/bits/local_lim.h
../occupancydevice..o: /usr/include/bits/mathcalls.h
../occupancydevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../occupancydevice..o: /usr/include/bits/posix1_lim.h
../occupancydevice..o: /usr/include/bits/posix2_lim.h
../occupancydevice..o: /usr/include/bits/pthreadtypes.h
../occupancydevice..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../occupancydevice..o: /usr/include/bits/sem.h /usr/include/bits/sigset.h
../occupancydevice..o: /usr/include/bits/sigthread.h
../occupancydevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../occupancydevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../occupancydevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../occupancydevice..o: /usr/include/bits/waitstatus.h
../occupancydevice..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../occupancydevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../occupancydevice..o: /usr/include/features.h /usr/include/_G_config.h
../occupancydevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
../occupancydevice..o: /usr/include/libio.h /usr/include/limits.h
../occupancydevice..o: /usr/include/linux/limits.h /usr/include/math.h
../occupancydevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
../occupancydevice..o: /usr/include/sched.h /usr/include/signal.h
../occupancydevice..o: /usr/include/stdint.h /usr/include/stdio.h
../occupancydevice..o: /usr/include/stdlib.h /usr/include/string.h
../occupancydevice..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../occupancydevice..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../occupancydevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../occupancydevice..o: /usr/include/sys/types.h /usr/include/time.h
../occupancydevice..o: /usr/include/wchar.h /usr/include/xlocale.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/fpos.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../occupancydevice..o: /usr/local/include/g++-v3/cstddef
../occupancydevice..o: /usr/local/include/g++-v3/cstdio
../occupancydevice..o: /usr/local/include/g++-v3/exception
../occupancydevice..o: /usr/local/include/g++-v3/exception_defines.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../occupancydevice..o: /usr/local/include/g++-v3/new
../occupancydevice..o: /usr/local/include/g++-v3/queue
../occupancydevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../occupancydevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../occupancydevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../occupancydevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../playerdevice..o: /home/vaughan/work/player/include/messages.h
../playerdevice..o: /home/vaughan/work/player/include/playercommon.h
../playerdevice..o: /home/vaughan/work/player/include/stage.h
../playerdevice..o: /usr/include/alloca.h /usr/include/asm/errno.h
../playerdevice..o: /usr/include/asm/sigcontext.h /usr/include/asm/socket.h
../playerdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../playerdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../playerdevice..o: /usr/include/bits/endian.h
../playerdevice..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
../playerdevice..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
../playerdevice..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
../playerdevice..o: /usr/include/bits/ipc.h /usr/include/bits/locale.h
../playerdevice..o: /usr/include/bits/local_lim.h
../playerdevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../playerdevice..o: /usr/include/bits/mman.h /usr/include/bits/nan.h
../playerdevice..o: /usr/include/bits/posix1_lim.h
../playerdevice..o: /usr/include/bits/posix2_lim.h
../playerdevice..o: /usr/include/bits/posix_opt.h
../playerdevice..o: /usr/include/bits/pthreadtypes.h
../playerdevice..o: /usr/include/bits/resource.h /usr/include/bits/sched.h
../playerdevice..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../playerdevice..o: /usr/include/bits/sigaction.h
../playerdevice..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
../playerdevice..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
../playerdevice..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
../playerdevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../playerdevice..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
../playerdevice..o: /usr/include/bits/termios.h /usr/include/bits/time.h
../playerdevice..o: /usr/include/bits/types.h /usr/include/bits/uio.h
../playerdevice..o: /usr/include/bits/waitflags.h
../playerdevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../playerdevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../playerdevice..o: /usr/include/ctype.h /usr/include/endian.h
../playerdevice..o: /usr/include/errno.h /usr/include/fcntl.h
../playerdevice..o: /usr/include/features.h /usr/include/_G_config.h
../playerdevice..o: /usr/include/gconv.h /usr/include/getopt.h
../playerdevice..o: /usr/include/gnu/stubs.h /usr/include/iconv.h
../playerdevice..o: /usr/include/langinfo.h /usr/include/libio.h
../playerdevice..o: /usr/include/limits.h /usr/include/linux/errno.h
../playerdevice..o: /usr/include/linux/limits.h /usr/include/locale.h
../playerdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../playerdevice..o: /usr/include/nl_types.h /usr/include/pthread.h
../playerdevice..o: /usr/include/sched.h /usr/include/signal.h
../playerdevice..o: /usr/include/stdint.h /usr/include/stdio.h
../playerdevice..o: /usr/include/stdlib.h /usr/include/string.h
../playerdevice..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../playerdevice..o: /usr/include/sys/mman.h /usr/include/sys/resource.h
../playerdevice..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../playerdevice..o: /usr/include/sys/socket.h /usr/include/sys/stat.h
../playerdevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../playerdevice..o: /usr/include/sys/ttydefaults.h /usr/include/sys/types.h
../playerdevice..o: /usr/include/sys/ucontext.h /usr/include/sys/uio.h
../playerdevice..o: /usr/include/sys/wait.h /usr/include/termios.h
../playerdevice..o: /usr/include/time.h /usr/include/ucontext.h
../playerdevice..o: /usr/include/unistd.h /usr/include/wchar.h
../playerdevice..o: /usr/include/wctype.h /usr/include/xlocale.h
../playerdevice..o: /usr/local/include/g++-v3/backward/backward_warning.h
../playerdevice..o: /usr/local/include/g++-v3/backward/iomanip.h
../playerdevice..o: /usr/local/include/g++-v3/backward/iostream.h
../playerdevice..o: /usr/local/include/g++-v3/backward/strstream
../playerdevice..o: /usr/local/include/g++-v3/backward/strstream.h
../playerdevice..o: /usr/local/include/g++-v3/bits/basic_ios.h
../playerdevice..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../playerdevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../playerdevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../playerdevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../playerdevice..o: /usr/local/include/g++-v3/bits/codecvt.h
../playerdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../playerdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../playerdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../playerdevice..o: /usr/local/include/g++-v3/bits/ios_base.h
../playerdevice..o: /usr/local/include/g++-v3/bits/istream.tcc
../playerdevice..o: /usr/local/include/g++-v3/bits/locale_facets.h
../playerdevice..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../playerdevice..o: /usr/local/include/g++-v3/bits/localefwd.h
../playerdevice..o: /usr/local/include/g++-v3/bits/ostream.tcc
../playerdevice..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_cctype.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_clocale.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_ctime.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_functional.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_iomanip.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_ios.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_iostream.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_istream.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_locale.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_ostream.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_string.h
../playerdevice..o: /usr/local/include/g++-v3/bits/std_vector.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../playerdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../playerdevice..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../playerdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../playerdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../playerdevice..o: /usr/local/include/g++-v3/cstddef
../playerdevice..o: /usr/local/include/g++-v3/cstdio
../playerdevice..o: /usr/local/include/g++-v3/exception
../playerdevice..o: /usr/local/include/g++-v3/exception_defines.h
../playerdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../playerdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../playerdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../playerdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../playerdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../playerdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../playerdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../playerdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../playerdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../playerdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../playerdevice..o: /usr/local/include/g++-v3/new
../playerdevice..o: /usr/local/include/g++-v3/queue
../playerdevice..o: /usr/local/include/g++-v3/typeinfo
../playerdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../playerdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../playerdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../playerdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../playerdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../player_xpsi..o: /usr/include/alloca.h /usr/include/asm/errno.h
../player_xpsi..o: /usr/include/asm/sigcontext.h /usr/include/assert.h
../player_xpsi..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../player_xpsi..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
../player_xpsi..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
../player_xpsi..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
../player_xpsi..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../player_xpsi..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../player_xpsi..o: /usr/include/bits/posix2_lim.h
../player_xpsi..o: /usr/include/bits/posix_opt.h
../player_xpsi..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../player_xpsi..o: /usr/include/bits/select.h /usr/include/bits/sigaction.h
../player_xpsi..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
../player_xpsi..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
../player_xpsi..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
../player_xpsi..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
../player_xpsi..o: /usr/include/bits/time.h /usr/include/bits/types.h
../player_xpsi..o: /usr/include/bits/waitflags.h
../player_xpsi..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../player_xpsi..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../player_xpsi..o: /usr/include/ctype.h /usr/include/endian.h
../player_xpsi..o: /usr/include/errno.h /usr/include/fcntl.h
../player_xpsi..o: /usr/include/features.h /usr/include/_G_config.h
../player_xpsi..o: /usr/include/gconv.h /usr/include/getopt.h
../player_xpsi..o: /usr/include/gnu/stubs.h /usr/include/iconv.h
../player_xpsi..o: /usr/include/langinfo.h /usr/include/libio.h
../player_xpsi..o: /usr/include/limits.h /usr/include/linux/errno.h
../player_xpsi..o: /usr/include/linux/limits.h /usr/include/locale.h
../player_xpsi..o: /usr/include/math.h /usr/include/nl_types.h
../player_xpsi..o: /usr/include/signal.h /usr/include/stdio.h
../player_xpsi..o: /usr/include/stdlib.h /usr/include/string.h
../player_xpsi..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
../player_xpsi..o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
../player_xpsi..o: /usr/include/sys/time.h /usr/include/sys/types.h
../player_xpsi..o: /usr/include/sys/ucontext.h /usr/include/time.h
../player_xpsi..o: /usr/include/ucontext.h /usr/include/unistd.h
../player_xpsi..o: /usr/include/values.h /usr/include/wchar.h
../player_xpsi..o: /usr/include/wctype.h /usr/include/X11/keysymdef.h
../player_xpsi..o: /usr/include/X11/keysym.h /usr/include/X11/Xfuncproto.h
../player_xpsi..o: /usr/include/X11/X.h /usr/include/X11/Xutil.h
../player_xpsi..o: /usr/include/xlocale.h
../player_xpsi..o: /usr/local/include/g++-v3/backward/backward_warning.h
../player_xpsi..o: /usr/local/include/g++-v3/backward/fstream.h
../player_xpsi..o: /usr/local/include/g++-v3/backward/iomanip.h
../player_xpsi..o: /usr/local/include/g++-v3/backward/iostream.h
../player_xpsi..o: /usr/local/include/g++-v3/backward/stream.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/basic_file.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/basic_ios.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../player_xpsi..o: /usr/local/include/g++-v3/bits/basic_string.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../player_xpsi..o: /usr/local/include/g++-v3/bits/char_traits.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/codecvt.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/concept_check.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/fpos.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/fstream.tcc
../player_xpsi..o: /usr/local/include/g++-v3/bits/functexcept.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/ios_base.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/istream.tcc
../player_xpsi..o: /usr/local/include/g++-v3/bits/locale_facets.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../player_xpsi..o: /usr/local/include/g++-v3/bits/localefwd.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/ostream.tcc
../player_xpsi..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_cassert.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_cctype.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_climits.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_clocale.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_cstring.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_ctime.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_fstream.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_functional.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_iomanip.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_ios.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_iostream.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_istream.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_iterator.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_locale.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_memory.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_ostream.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_string.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/std_vector.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_algo.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_construct.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_function.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_heap.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_pair.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_threads.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/stl_vector.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../player_xpsi..o: /usr/local/include/g++-v3/bits/stringfwd.h
../player_xpsi..o: /usr/local/include/g++-v3/bits/type_traits.h
../player_xpsi..o: /usr/local/include/g++-v3/cstddef
../player_xpsi..o: /usr/local/include/g++-v3/cstdio
../player_xpsi..o: /usr/local/include/g++-v3/exception
../player_xpsi..o: /usr/local/include/g++-v3/exception_defines.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/basic_file_model.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../player_xpsi..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../player_xpsi..o: /usr/local/include/g++-v3/new
../player_xpsi..o: /usr/local/include/g++-v3/typeinfo
../player_xpsi..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../player_xpsi..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../player_xpsi..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../player_xpsi..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../player_xpsi..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../player_xpsi..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xarch.h
../player_xpsi..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xlib.h
../player_xpsi..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xosdefs.h
../player_xpsi..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xos.h
../positiondevice..o: /home/vaughan/work/player/include/messages.h
../positiondevice..o: /home/vaughan/work/player/include/playercommon.h
../positiondevice..o: /home/vaughan/work/player/include/stage.h
../positiondevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../positiondevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../positiondevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../positiondevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../positiondevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../positiondevice..o: /usr/include/bits/local_lim.h
../positiondevice..o: /usr/include/bits/mathcalls.h
../positiondevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../positiondevice..o: /usr/include/bits/posix1_lim.h
../positiondevice..o: /usr/include/bits/posix2_lim.h
../positiondevice..o: /usr/include/bits/pthreadtypes.h
../positiondevice..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../positiondevice..o: /usr/include/bits/sem.h /usr/include/bits/sigset.h
../positiondevice..o: /usr/include/bits/sigthread.h
../positiondevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../positiondevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../positiondevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../positiondevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../positiondevice..o: /usr/include/bits/wordsize.h
../positiondevice..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../positiondevice..o: /usr/include/features.h /usr/include/_G_config.h
../positiondevice..o: /usr/include/gconv.h /usr/include/gnu/stubs.h
../positiondevice..o: /usr/include/libio.h /usr/include/limits.h
../positiondevice..o: /usr/include/linux/limits.h /usr/include/math.h
../positiondevice..o: /usr/include/netinet/in.h /usr/include/pthread.h
../positiondevice..o: /usr/include/sched.h /usr/include/signal.h
../positiondevice..o: /usr/include/stdint.h /usr/include/stdio.h
../positiondevice..o: /usr/include/stdlib.h /usr/include/string.h
../positiondevice..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../positiondevice..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../positiondevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../positiondevice..o: /usr/include/sys/types.h /usr/include/time.h
../positiondevice..o: /usr/include/wchar.h /usr/include/xlocale.h
../positiondevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../positiondevice..o: /usr/local/include/g++-v3/bits/fpos.h
../positiondevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../positiondevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../positiondevice..o: /usr/local/include/g++-v3/cstddef
../positiondevice..o: /usr/local/include/g++-v3/cstdio
../positiondevice..o: /usr/local/include/g++-v3/exception
../positiondevice..o: /usr/local/include/g++-v3/exception_defines.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../positiondevice..o: /usr/local/include/g++-v3/new
../positiondevice..o: /usr/local/include/g++-v3/queue
../positiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../positiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../positiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../positiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../ptzdevice..o: /home/vaughan/work/player/include/messages.h
../ptzdevice..o: /home/vaughan/work/player/include/playercommon.h
../ptzdevice..o: /home/vaughan/work/player/include/stage.h
../ptzdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../ptzdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../ptzdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../ptzdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../ptzdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../ptzdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../ptzdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../ptzdevice..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../ptzdevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../ptzdevice..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../ptzdevice..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../ptzdevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../ptzdevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../ptzdevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../ptzdevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../ptzdevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../ptzdevice..o: /usr/include/endian.h /usr/include/features.h
../ptzdevice..o: /usr/include/_G_config.h /usr/include/gconv.h
../ptzdevice..o: /usr/include/gnu/stubs.h /usr/include/libio.h
../ptzdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../ptzdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../ptzdevice..o: /usr/include/pthread.h /usr/include/sched.h
../ptzdevice..o: /usr/include/signal.h /usr/include/stdint.h
../ptzdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../ptzdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../ptzdevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../ptzdevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../ptzdevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../ptzdevice..o: /usr/include/time.h /usr/include/wchar.h
../ptzdevice..o: /usr/include/xlocale.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../ptzdevice..o: /usr/local/include/g++-v3/cstddef
../ptzdevice..o: /usr/local/include/g++-v3/cstdio
../ptzdevice..o: /usr/local/include/g++-v3/exception
../ptzdevice..o: /usr/local/include/g++-v3/exception_defines.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../ptzdevice..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../ptzdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../ptzdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../ptzdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../ptzdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../puck..o: /home/vaughan/work/player/include/messages.h
../puck..o: /home/vaughan/work/player/include/playercommon.h
../puck..o: /home/vaughan/work/player/include/stage.h /usr/include/alloca.h
../puck..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../puck..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../puck..o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
../puck..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
../puck..o: /usr/include/bits/ipc.h /usr/include/bits/local_lim.h
../puck..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../puck..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../puck..o: /usr/include/bits/posix2_lim.h /usr/include/bits/pthreadtypes.h
../puck..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../puck..o: /usr/include/bits/sem.h /usr/include/bits/sigset.h
../puck..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
../puck..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
../puck..o: /usr/include/bits/time.h /usr/include/bits/types.h
../puck..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
../puck..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../puck..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../puck..o: /usr/include/features.h /usr/include/_G_config.h
../puck..o: /usr/include/gconv.h /usr/include/gnu/stubs.h /usr/include/libio.h
../puck..o: /usr/include/limits.h /usr/include/linux/limits.h
../puck..o: /usr/include/math.h /usr/include/netinet/in.h
../puck..o: /usr/include/pthread.h /usr/include/sched.h /usr/include/signal.h
../puck..o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
../puck..o: /usr/include/string.h /usr/include/sys/cdefs.h
../puck..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../puck..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../puck..o: /usr/include/sys/time.h /usr/include/sys/types.h
../puck..o: /usr/include/time.h /usr/include/wchar.h /usr/include/xlocale.h
../puck..o: /usr/local/include/g++-v3/bits/concept_check.h
../puck..o: /usr/local/include/g++-v3/bits/fpos.h
../puck..o: /usr/local/include/g++-v3/bits/functexcept.h
../puck..o: /usr/local/include/g++-v3/bits/std_cassert.h
../puck..o: /usr/local/include/g++-v3/bits/std_climits.h
../puck..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../puck..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../puck..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../puck..o: /usr/local/include/g++-v3/bits/std_cstring.h
../puck..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../puck..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../puck..o: /usr/local/include/g++-v3/bits/std_queue.h
../puck..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../puck..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../puck..o: /usr/local/include/g++-v3/bits/stl_construct.h
../puck..o: /usr/local/include/g++-v3/bits/stl_deque.h
../puck..o: /usr/local/include/g++-v3/bits/stl_function.h
../puck..o: /usr/local/include/g++-v3/bits/stl_heap.h
../puck..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../puck..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../puck..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../puck..o: /usr/local/include/g++-v3/bits/stl_pair.h
../puck..o: /usr/local/include/g++-v3/bits/stl_queue.h
../puck..o: /usr/local/include/g++-v3/bits/stl_threads.h
../puck..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../puck..o: /usr/local/include/g++-v3/bits/stl_vector.h
../puck..o: /usr/local/include/g++-v3/bits/stringfwd.h
../puck..o: /usr/local/include/g++-v3/bits/type_traits.h
../puck..o: /usr/local/include/g++-v3/cstddef /usr/local/include/g++-v3/cstdio
../puck..o: /usr/local/include/g++-v3/exception
../puck..o: /usr/local/include/g++-v3/exception_defines.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../puck..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../puck..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../puck..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../puck..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../puck..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../render..o: /home/vaughan/work/player/include/messages.h
../render..o: /home/vaughan/work/player/include/playercommon.h
../render..o: /usr/include/alloca.h /usr/include/asm/errno.h
../render..o: /usr/include/assert.h /usr/include/bits/confname.h
../render..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../render..o: /usr/include/bits/errno.h /usr/include/bits/fcntl.h
../render..o: /usr/include/bits/huge_val.h /usr/include/bits/locale.h
../render..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../render..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../render..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../render..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
../render..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../render..o: /usr/include/bits/sigset.h /usr/include/bits/stat.h
../render..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../render..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../render..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../render..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../render..o: /usr/include/ctype.h /usr/include/endian.h /usr/include/errno.h
../render..o: /usr/include/fcntl.h /usr/include/features.h
../render..o: /usr/include/_G_config.h /usr/include/gconv.h
../render..o: /usr/include/getopt.h /usr/include/gnu/stubs.h
../render..o: /usr/include/iconv.h /usr/include/langinfo.h
../render..o: /usr/include/libio.h /usr/include/limits.h
../render..o: /usr/include/linux/errno.h /usr/include/linux/limits.h
../render..o: /usr/include/locale.h /usr/include/math.h
../render..o: /usr/include/nl_types.h /usr/include/stdint.h
../render..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
../render..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
../render..o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
../render..o: /usr/include/sys/time.h /usr/include/sys/types.h
../render..o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
../render..o: /usr/include/wctype.h /usr/include/X11/keysymdef.h
../render..o: /usr/include/X11/keysym.h /usr/include/X11/Xfuncproto.h
../render..o: /usr/include/X11/X.h /usr/include/X11/Xutil.h
../render..o: /usr/include/xlocale.h
../render..o: /usr/local/include/g++-v3/backward/backward_warning.h
../render..o: /usr/local/include/g++-v3/backward/iostream.h
../render..o: /usr/local/include/g++-v3/bits/basic_ios.h
../render..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../render..o: /usr/local/include/g++-v3/bits/basic_string.h
../render..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../render..o: /usr/local/include/g++-v3/bits/char_traits.h
../render..o: /usr/local/include/g++-v3/bits/codecvt.h
../render..o: /usr/local/include/g++-v3/bits/concept_check.h
../render..o: /usr/local/include/g++-v3/bits/fpos.h
../render..o: /usr/local/include/g++-v3/bits/functexcept.h
../render..o: /usr/local/include/g++-v3/bits/ios_base.h
../render..o: /usr/local/include/g++-v3/bits/istream.tcc
../render..o: /usr/local/include/g++-v3/bits/locale_facets.h
../render..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../render..o: /usr/local/include/g++-v3/bits/localefwd.h
../render..o: /usr/local/include/g++-v3/bits/ostream.tcc
../render..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../render..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../render..o: /usr/local/include/g++-v3/bits/std_cassert.h
../render..o: /usr/local/include/g++-v3/bits/std_cctype.h
../render..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../render..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../render..o: /usr/local/include/g++-v3/bits/std_climits.h
../render..o: /usr/local/include/g++-v3/bits/std_clocale.h
../render..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../render..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../render..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../render..o: /usr/local/include/g++-v3/bits/std_cstring.h
../render..o: /usr/local/include/g++-v3/bits/std_ctime.h
../render..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../render..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../render..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../render..o: /usr/local/include/g++-v3/bits/std_ios.h
../render..o: /usr/local/include/g++-v3/bits/std_iostream.h
../render..o: /usr/local/include/g++-v3/bits/std_istream.h
../render..o: /usr/local/include/g++-v3/bits/std_iterator.h
../render..o: /usr/local/include/g++-v3/bits/std_locale.h
../render..o: /usr/local/include/g++-v3/bits/std_memory.h
../render..o: /usr/local/include/g++-v3/bits/std_ostream.h
../render..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../render..o: /usr/local/include/g++-v3/bits/std_string.h
../render..o: /usr/local/include/g++-v3/bits/std_vector.h
../render..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../render..o: /usr/local/include/g++-v3/bits/stl_algo.h
../render..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../render..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../render..o: /usr/local/include/g++-v3/bits/stl_construct.h
../render..o: /usr/local/include/g++-v3/bits/stl_heap.h
../render..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../render..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../render..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../render..o: /usr/local/include/g++-v3/bits/stl_pair.h
../render..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../render..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../render..o: /usr/local/include/g++-v3/bits/stl_threads.h
../render..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../render..o: /usr/local/include/g++-v3/bits/stl_vector.h
../render..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../render..o: /usr/local/include/g++-v3/bits/stringfwd.h
../render..o: /usr/local/include/g++-v3/bits/type_traits.h
../render..o: /usr/local/include/g++-v3/cstddef
../render..o: /usr/local/include/g++-v3/cstdio
../render..o: /usr/local/include/g++-v3/exception
../render..o: /usr/local/include/g++-v3/exception_defines.h
../render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../render..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/typeinfo
../render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xarch.h
../render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xlib.h
../render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xosdefs.h
../render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xos.h
../rtk_main..o: /home/vaughan/work/player/include/messages.h
../rtk_main..o: /home/vaughan/work/player/include/playercommon.h
../rtk_main..o: /home/vaughan/work/player/include/stage.h
../rtk_main..o: /usr/include/alloca.h /usr/include/asm/socket.h
../rtk_main..o: /usr/include/asm/sockios.h /usr/include/assert.h
../rtk_main..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../rtk_main..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../rtk_main..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../rtk_main..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../rtk_main..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../rtk_main..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../rtk_main..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../rtk_main..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../rtk_main..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../rtk_main..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../rtk_main..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../rtk_main..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../rtk_main..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../rtk_main..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../rtk_main..o: /usr/include/endian.h /usr/include/features.h
../rtk_main..o: /usr/include/_G_config.h /usr/include/gconv.h
../rtk_main..o: /usr/include/gnu/stubs.h /usr/include/libio.h
../rtk_main..o: /usr/include/limits.h /usr/include/linux/limits.h
../rtk_main..o: /usr/include/math.h /usr/include/netinet/in.h
../rtk_main..o: /usr/include/pthread.h /usr/include/sched.h
../rtk_main..o: /usr/include/signal.h /usr/include/stdint.h
../rtk_main..o: /usr/include/stdio.h /usr/include/stdlib.h
../rtk_main..o: /usr/include/string.h /usr/include/sys/cdefs.h
../rtk_main..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../rtk_main..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../rtk_main..o: /usr/include/sys/time.h /usr/include/sys/types.h
../rtk_main..o: /usr/include/time.h /usr/include/wchar.h
../rtk_main..o: /usr/include/xlocale.h
../rtk_main..o: /usr/local/include/g++-v3/bits/concept_check.h
../rtk_main..o: /usr/local/include/g++-v3/bits/fpos.h
../rtk_main..o: /usr/local/include/g++-v3/bits/functexcept.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cassert.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_climits.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cstring.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_queue.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_construct.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_deque.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_function.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_heap.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_pair.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_queue.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_threads.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_vector.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stringfwd.h
../rtk_main..o: /usr/local/include/g++-v3/bits/type_traits.h
../rtk_main..o: /usr/local/include/g++-v3/cstddef
../rtk_main..o: /usr/local/include/g++-v3/cstdio
../rtk_main..o: /usr/local/include/g++-v3/exception
../rtk_main..o: /usr/local/include/g++-v3/exception_defines.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../rtk_main..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../rtk_main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../rtk_main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../rtk_main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../rtk_main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../sonardevice..o: /home/vaughan/work/player/include/messages.h
../sonardevice..o: /home/vaughan/work/player/include/playercommon.h
../sonardevice..o: /home/vaughan/work/player/include/stage.h
../sonardevice..o: /usr/include/alloca.h /usr/include/asm/errno.h
../sonardevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../sonardevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../sonardevice..o: /usr/include/bits/endian.h /usr/include/bits/errno.h
../sonardevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../sonardevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../sonardevice..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
../sonardevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../sonardevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../sonardevice..o: /usr/include/bits/posix2_lim.h
../sonardevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../sonardevice..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../sonardevice..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../sonardevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../sonardevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../sonardevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../sonardevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../sonardevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../sonardevice..o: /usr/include/ctype.h /usr/include/endian.h
../sonardevice..o: /usr/include/errno.h /usr/include/features.h
../sonardevice..o: /usr/include/_G_config.h /usr/include/gconv.h
../sonardevice..o: /usr/include/gnu/stubs.h /usr/include/iconv.h
../sonardevice..o: /usr/include/langinfo.h /usr/include/libio.h
../sonardevice..o: /usr/include/limits.h /usr/include/linux/errno.h
../sonardevice..o: /usr/include/linux/limits.h /usr/include/locale.h
../sonardevice..o: /usr/include/math.h /usr/include/netinet/in.h
../sonardevice..o: /usr/include/nl_types.h /usr/include/pthread.h
../sonardevice..o: /usr/include/sched.h /usr/include/signal.h
../sonardevice..o: /usr/include/stdint.h /usr/include/stdio.h
../sonardevice..o: /usr/include/stdlib.h /usr/include/string.h
../sonardevice..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../sonardevice..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../sonardevice..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../sonardevice..o: /usr/include/sys/types.h /usr/include/time.h
../sonardevice..o: /usr/include/wchar.h /usr/include/wctype.h
../sonardevice..o: /usr/include/xlocale.h
../sonardevice..o: /usr/local/include/g++-v3/backward/backward_warning.h
../sonardevice..o: /usr/local/include/g++-v3/backward/iostream.h
../sonardevice..o: /usr/local/include/g++-v3/bits/basic_ios.h
../sonardevice..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../sonardevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../sonardevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../sonardevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../sonardevice..o: /usr/local/include/g++-v3/bits/codecvt.h
../sonardevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../sonardevice..o: /usr/local/include/g++-v3/bits/fpos.h
../sonardevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../sonardevice..o: /usr/local/include/g++-v3/bits/ios_base.h
../sonardevice..o: /usr/local/include/g++-v3/bits/istream.tcc
../sonardevice..o: /usr/local/include/g++-v3/bits/locale_facets.h
../sonardevice..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../sonardevice..o: /usr/local/include/g++-v3/bits/localefwd.h
../sonardevice..o: /usr/local/include/g++-v3/bits/ostream.tcc
../sonardevice..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_cctype.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_clocale.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_ctime.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_ios.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_iostream.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_istream.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_locale.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_ostream.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_string.h
../sonardevice..o: /usr/local/include/g++-v3/bits/std_vector.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../sonardevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../sonardevice..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../sonardevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../sonardevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../sonardevice..o: /usr/local/include/g++-v3/cstddef
../sonardevice..o: /usr/local/include/g++-v3/cstdio
../sonardevice..o: /usr/local/include/g++-v3/exception
../sonardevice..o: /usr/local/include/g++-v3/exception_defines.h
../sonardevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../sonardevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../sonardevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../sonardevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../sonardevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../sonardevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../sonardevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../sonardevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../sonardevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../sonardevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../sonardevice..o: /usr/local/include/g++-v3/new
../sonardevice..o: /usr/local/include/g++-v3/queue
../sonardevice..o: /usr/local/include/g++-v3/typeinfo
../sonardevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../sonardevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../sonardevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../sonardevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../sonardevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../truthdevice..o: /home/vaughan/work/player/include/messages.h
../truthdevice..o: /home/vaughan/work/player/include/playercommon.h
../truthdevice..o: /home/vaughan/work/player/include/stage.h
../truthdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../truthdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../truthdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../truthdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../truthdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../truthdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../truthdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../truthdevice..o: /usr/include/bits/posix1_lim.h
../truthdevice..o: /usr/include/bits/posix2_lim.h
../truthdevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../truthdevice..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../truthdevice..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../truthdevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../truthdevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../truthdevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../truthdevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../truthdevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../truthdevice..o: /usr/include/endian.h /usr/include/features.h
../truthdevice..o: /usr/include/_G_config.h /usr/include/gconv.h
../truthdevice..o: /usr/include/gnu/stubs.h /usr/include/libio.h
../truthdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../truthdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../truthdevice..o: /usr/include/pthread.h /usr/include/sched.h
../truthdevice..o: /usr/include/signal.h /usr/include/stdint.h
../truthdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../truthdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../truthdevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../truthdevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../truthdevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../truthdevice..o: /usr/include/time.h /usr/include/wchar.h
../truthdevice..o: /usr/include/xlocale.h
../truthdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../truthdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../truthdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../truthdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../truthdevice..o: /usr/local/include/g++-v3/cstddef
../truthdevice..o: /usr/local/include/g++-v3/cstdio
../truthdevice..o: /usr/local/include/g++-v3/exception
../truthdevice..o: /usr/local/include/g++-v3/exception_defines.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../truthdevice..o: /usr/local/include/g++-v3/new
../truthdevice..o: /usr/local/include/g++-v3/queue
../truthdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../truthdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../truthdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../truthdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../truthlog..o: /home/vaughan/work/player/include/messages.h
../truthlog..o: /home/vaughan/work/player/include/playercommon.h
../truthlog..o: /usr/include/alloca.h /usr/include/arpa/inet.h
../truthlog..o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
../truthlog..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../truthlog..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../truthlog..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../truthlog..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
../truthlog..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
../truthlog..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
../truthlog..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
../truthlog..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../truthlog..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../truthlog..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
../truthlog..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../truthlog..o: /usr/include/bits/select.h /usr/include/bits/sigaction.h
../truthlog..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
../truthlog..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
../truthlog..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
../truthlog..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../truthlog..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
../truthlog..o: /usr/include/bits/time.h /usr/include/bits/types.h
../truthlog..o: /usr/include/bits/uio.h /usr/include/bits/waitflags.h
../truthlog..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../truthlog..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../truthlog..o: /usr/include/ctype.h /usr/include/endian.h
../truthlog..o: /usr/include/errno.h /usr/include/fcntl.h
../truthlog..o: /usr/include/features.h /usr/include/_G_config.h
../truthlog..o: /usr/include/gconv.h /usr/include/getopt.h
../truthlog..o: /usr/include/gnu/stubs.h /usr/include/iconv.h
../truthlog..o: /usr/include/langinfo.h /usr/include/libio.h
../truthlog..o: /usr/include/limits.h /usr/include/linux/errno.h
../truthlog..o: /usr/include/linux/limits.h /usr/include/locale.h
../truthlog..o: /usr/include/math.h /usr/include/netinet/in.h
../truthlog..o: /usr/include/nl_types.h /usr/include/pthread.h
../truthlog..o: /usr/include/sched.h /usr/include/signal.h
../truthlog..o: /usr/include/stdint.h /usr/include/stdio.h
../truthlog..o: /usr/include/stdlib.h /usr/include/string.h
../truthlog..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
../truthlog..o: /usr/include/sys/socket.h /usr/include/sys/stat.h
../truthlog..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../truthlog..o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
../truthlog..o: /usr/include/sys/uio.h /usr/include/time.h
../truthlog..o: /usr/include/ucontext.h /usr/include/unistd.h
../truthlog..o: /usr/include/values.h /usr/include/wchar.h
../truthlog..o: /usr/include/wctype.h /usr/include/xlocale.h
../truthlog..o: /usr/local/include/g++-v3/backward/backward_warning.h
../truthlog..o: /usr/local/include/g++-v3/backward/iostream.h
../truthlog..o: /usr/local/include/g++-v3/bits/basic_ios.h
../truthlog..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../truthlog..o: /usr/local/include/g++-v3/bits/basic_string.h
../truthlog..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../truthlog..o: /usr/local/include/g++-v3/bits/char_traits.h
../truthlog..o: /usr/local/include/g++-v3/bits/codecvt.h
../truthlog..o: /usr/local/include/g++-v3/bits/concept_check.h
../truthlog..o: /usr/local/include/g++-v3/bits/fpos.h
../truthlog..o: /usr/local/include/g++-v3/bits/functexcept.h
../truthlog..o: /usr/local/include/g++-v3/bits/ios_base.h
../truthlog..o: /usr/local/include/g++-v3/bits/istream.tcc
../truthlog..o: /usr/local/include/g++-v3/bits/locale_facets.h
../truthlog..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../truthlog..o: /usr/local/include/g++-v3/bits/localefwd.h
../truthlog..o: /usr/local/include/g++-v3/bits/ostream.tcc
../truthlog..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_cassert.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_cctype.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_climits.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_clocale.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_cstring.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_ctime.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_ios.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_iostream.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_istream.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_iterator.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_locale.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_memory.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_ostream.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_string.h
../truthlog..o: /usr/local/include/g++-v3/bits/std_vector.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_algo.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_construct.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_heap.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_pair.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_threads.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../truthlog..o: /usr/local/include/g++-v3/bits/stl_vector.h
../truthlog..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../truthlog..o: /usr/local/include/g++-v3/bits/stringfwd.h
../truthlog..o: /usr/local/include/g++-v3/bits/type_traits.h
../truthlog..o: /usr/local/include/g++-v3/cstddef
../truthlog..o: /usr/local/include/g++-v3/cstdio
../truthlog..o: /usr/local/include/g++-v3/exception
../truthlog..o: /usr/local/include/g++-v3/exception_defines.h
../truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../truthlog..o: /usr/local/include/g++-v3/new
../truthlog..o: /usr/local/include/g++-v3/typeinfo
../truthlog..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../truthlog..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../truthlog..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../truthlog..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../truthlog..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../truthserver..o: /home/vaughan/work/player/include/messages.h
../truthserver..o: /home/vaughan/work/player/include/playercommon.h
../truthserver..o: /home/vaughan/work/player/include/stage.h
../truthserver..o: /usr/include/alloca.h /usr/include/arpa/inet.h
../truthserver..o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
../truthserver..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../truthserver..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../truthserver..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../truthserver..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
../truthserver..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
../truthserver..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
../truthserver..o: /usr/include/bits/ipc.h /usr/include/bits/locale.h
../truthserver..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../truthserver..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../truthserver..o: /usr/include/bits/netdb.h /usr/include/bits/posix1_lim.h
../truthserver..o: /usr/include/bits/posix2_lim.h
../truthserver..o: /usr/include/bits/posix_opt.h
../truthserver..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../truthserver..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../truthserver..o: /usr/include/bits/sigaction.h
../truthserver..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
../truthserver..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
../truthserver..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
../truthserver..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../truthserver..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
../truthserver..o: /usr/include/bits/termios.h /usr/include/bits/time.h
../truthserver..o: /usr/include/bits/types.h /usr/include/bits/uio.h
../truthserver..o: /usr/include/bits/waitflags.h
../truthserver..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../truthserver..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../truthserver..o: /usr/include/ctype.h /usr/include/endian.h
../truthserver..o: /usr/include/errno.h /usr/include/fcntl.h
../truthserver..o: /usr/include/features.h /usr/include/_G_config.h
../truthserver..o: /usr/include/gconv.h /usr/include/getopt.h
../truthserver..o: /usr/include/gnu/stubs.h /usr/include/iconv.h
../truthserver..o: /usr/include/langinfo.h /usr/include/libio.h
../truthserver..o: /usr/include/limits.h /usr/include/linux/errno.h
../truthserver..o: /usr/include/linux/limits.h /usr/include/locale.h
../truthserver..o: /usr/include/math.h /usr/include/netdb.h
../truthserver..o: /usr/include/netinet/in.h /usr/include/nl_types.h
../truthserver..o: /usr/include/pthread.h /usr/include/rpc/netdb.h
../truthserver..o: /usr/include/sched.h /usr/include/signal.h
../truthserver..o: /usr/include/stdint.h /usr/include/stdio.h
../truthserver..o: /usr/include/stdlib.h /usr/include/string.h
../truthserver..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../truthserver..o: /usr/include/sys/select.h /usr/include/sys/sem.h
../truthserver..o: /usr/include/sys/socket.h /usr/include/sys/stat.h
../truthserver..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../truthserver..o: /usr/include/sys/ttydefaults.h /usr/include/sys/types.h
../truthserver..o: /usr/include/sys/ucontext.h /usr/include/sys/uio.h
../truthserver..o: /usr/include/termios.h /usr/include/time.h
../truthserver..o: /usr/include/ucontext.h /usr/include/unistd.h
../truthserver..o: /usr/include/wchar.h /usr/include/wctype.h
../truthserver..o: /usr/include/xlocale.h
../truthserver..o: /usr/local/include/g++-v3/backward/backward_warning.h
../truthserver..o: /usr/local/include/g++-v3/backward/iomanip.h
../truthserver..o: /usr/local/include/g++-v3/backward/iostream.h
../truthserver..o: /usr/local/include/g++-v3/backward/strstream
../truthserver..o: /usr/local/include/g++-v3/backward/strstream.h
../truthserver..o: /usr/local/include/g++-v3/bits/basic_ios.h
../truthserver..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../truthserver..o: /usr/local/include/g++-v3/bits/basic_string.h
../truthserver..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../truthserver..o: /usr/local/include/g++-v3/bits/char_traits.h
../truthserver..o: /usr/local/include/g++-v3/bits/codecvt.h
../truthserver..o: /usr/local/include/g++-v3/bits/concept_check.h
../truthserver..o: /usr/local/include/g++-v3/bits/fpos.h
../truthserver..o: /usr/local/include/g++-v3/bits/functexcept.h
../truthserver..o: /usr/local/include/g++-v3/bits/ios_base.h
../truthserver..o: /usr/local/include/g++-v3/bits/istream.tcc
../truthserver..o: /usr/local/include/g++-v3/bits/locale_facets.h
../truthserver..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../truthserver..o: /usr/local/include/g++-v3/bits/localefwd.h
../truthserver..o: /usr/local/include/g++-v3/bits/ostream.tcc
../truthserver..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_cassert.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_cctype.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_climits.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_clocale.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_cstring.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_ctime.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_functional.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_iomanip.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_ios.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_iostream.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_istream.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_iterator.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_locale.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_memory.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_ostream.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_queue.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_string.h
../truthserver..o: /usr/local/include/g++-v3/bits/std_vector.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_algo.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_construct.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_deque.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_function.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_heap.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_pair.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_queue.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_threads.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../truthserver..o: /usr/local/include/g++-v3/bits/stl_vector.h
../truthserver..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../truthserver..o: /usr/local/include/g++-v3/bits/stringfwd.h
../truthserver..o: /usr/local/include/g++-v3/bits/type_traits.h
../truthserver..o: /usr/local/include/g++-v3/cstddef
../truthserver..o: /usr/local/include/g++-v3/cstdio
../truthserver..o: /usr/local/include/g++-v3/exception
../truthserver..o: /usr/local/include/g++-v3/exception_defines.h
../truthserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../truthserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../truthserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../truthserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../truthserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../truthserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../truthserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../truthserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../truthserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../truthserver..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../truthserver..o: /usr/local/include/g++-v3/new
../truthserver..o: /usr/local/include/g++-v3/queue
../truthserver..o: /usr/local/include/g++-v3/typeinfo
../truthserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../truthserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../truthserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../truthserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../truthserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../visionbeacon..o: /home/vaughan/work/player/include/messages.h
../visionbeacon..o: /home/vaughan/work/player/include/playercommon.h
../visionbeacon..o: /home/vaughan/work/player/include/stage.h
../visionbeacon..o: /usr/include/alloca.h /usr/include/asm/socket.h
../visionbeacon..o: /usr/include/asm/sockios.h /usr/include/assert.h
../visionbeacon..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../visionbeacon..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../visionbeacon..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../visionbeacon..o: /usr/include/bits/local_lim.h
../visionbeacon..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../visionbeacon..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../visionbeacon..o: /usr/include/bits/posix2_lim.h
../visionbeacon..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../visionbeacon..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../visionbeacon..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../visionbeacon..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../visionbeacon..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../visionbeacon..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../visionbeacon..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../visionbeacon..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../visionbeacon..o: /usr/include/endian.h /usr/include/features.h
../visionbeacon..o: /usr/include/_G_config.h /usr/include/gconv.h
../visionbeacon..o: /usr/include/gnu/stubs.h /usr/include/libio.h
../visionbeacon..o: /usr/include/limits.h /usr/include/linux/limits.h
../visionbeacon..o: /usr/include/math.h /usr/include/netinet/in.h
../visionbeacon..o: /usr/include/pthread.h /usr/include/sched.h
../visionbeacon..o: /usr/include/signal.h /usr/include/stdint.h
../visionbeacon..o: /usr/include/stdio.h /usr/include/stdlib.h
../visionbeacon..o: /usr/include/string.h /usr/include/sys/cdefs.h
../visionbeacon..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../visionbeacon..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../visionbeacon..o: /usr/include/sys/time.h /usr/include/sys/types.h
../visionbeacon..o: /usr/include/time.h /usr/include/wchar.h
../visionbeacon..o: /usr/include/xlocale.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/concept_check.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/fpos.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/functexcept.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cassert.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_climits.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cstring.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_queue.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_construct.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_deque.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_function.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_heap.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_pair.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_queue.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_threads.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_vector.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stringfwd.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/type_traits.h
../visionbeacon..o: /usr/local/include/g++-v3/cstddef
../visionbeacon..o: /usr/local/include/g++-v3/cstdio
../visionbeacon..o: /usr/local/include/g++-v3/exception
../visionbeacon..o: /usr/local/include/g++-v3/exception_defines.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../visionbeacon..o: /usr/local/include/g++-v3/new
../visionbeacon..o: /usr/local/include/g++-v3/queue
../visionbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../visionbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../visionbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../visionbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../visiondevice..o: /home/vaughan/work/player/include/messages.h
../visiondevice..o: /home/vaughan/work/player/include/playercommon.h
../visiondevice..o: /home/vaughan/work/player/include/stage.h
../visiondevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../visiondevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../visiondevice..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../visiondevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../visiondevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../visiondevice..o: /usr/include/bits/local_lim.h
../visiondevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../visiondevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../visiondevice..o: /usr/include/bits/posix2_lim.h
../visiondevice..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../visiondevice..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../visiondevice..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../visiondevice..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../visiondevice..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../visiondevice..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../visiondevice..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../visiondevice..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../visiondevice..o: /usr/include/endian.h /usr/include/features.h
../visiondevice..o: /usr/include/_G_config.h /usr/include/gconv.h
../visiondevice..o: /usr/include/gnu/stubs.h /usr/include/libio.h
../visiondevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../visiondevice..o: /usr/include/math.h /usr/include/netinet/in.h
../visiondevice..o: /usr/include/pthread.h /usr/include/sched.h
../visiondevice..o: /usr/include/signal.h /usr/include/stdint.h
../visiondevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../visiondevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../visiondevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../visiondevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../visiondevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../visiondevice..o: /usr/include/time.h /usr/include/wchar.h
../visiondevice..o: /usr/include/xlocale.h
../visiondevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../visiondevice..o: /usr/local/include/g++-v3/bits/fpos.h
../visiondevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_construct.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_deque.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_function.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_heap.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_pair.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_queue.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../visiondevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../visiondevice..o: /usr/local/include/g++-v3/cstddef
../visiondevice..o: /usr/local/include/g++-v3/cstdio
../visiondevice..o: /usr/local/include/g++-v3/exception
../visiondevice..o: /usr/local/include/g++-v3/exception_defines.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../visiondevice..o: /usr/local/include/g++-v3/new
../visiondevice..o: /usr/local/include/g++-v3/queue
../visiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../visiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../visiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../visiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../world_load..o: /home/vaughan/work/player/include/messages.h
../world_load..o: /home/vaughan/work/player/include/playercommon.h
../world_load..o: /home/vaughan/work/player/include/stage.h
../world_load..o: /usr/include/alloca.h /usr/include/asm/socket.h
../world_load..o: /usr/include/asm/sockios.h /usr/include/assert.h
../world_load..o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
../world_load..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../world_load..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../world_load..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../world_load..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../world_load..o: /usr/include/bits/posix1_lim.h
../world_load..o: /usr/include/bits/posix2_lim.h
../world_load..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../world_load..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../world_load..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../world_load..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../world_load..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../world_load..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../world_load..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../world_load..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../world_load..o: /usr/include/endian.h /usr/include/features.h
../world_load..o: /usr/include/_G_config.h /usr/include/gconv.h
../world_load..o: /usr/include/gnu/stubs.h /usr/include/libio.h
../world_load..o: /usr/include/limits.h /usr/include/linux/limits.h
../world_load..o: /usr/include/math.h /usr/include/netinet/in.h
../world_load..o: /usr/include/pthread.h /usr/include/sched.h
../world_load..o: /usr/include/signal.h /usr/include/stdint.h
../world_load..o: /usr/include/stdio.h /usr/include/stdlib.h
../world_load..o: /usr/include/string.h /usr/include/sys/cdefs.h
../world_load..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../world_load..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../world_load..o: /usr/include/sys/time.h /usr/include/sys/types.h
../world_load..o: /usr/include/time.h /usr/include/wchar.h
../world_load..o: /usr/include/xlocale.h
../world_load..o: /usr/local/include/g++-v3/bits/concept_check.h
../world_load..o: /usr/local/include/g++-v3/bits/fpos.h
../world_load..o: /usr/local/include/g++-v3/bits/functexcept.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cassert.h
../world_load..o: /usr/local/include/g++-v3/bits/std_climits.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cstring.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../world_load..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../world_load..o: /usr/local/include/g++-v3/bits/std_queue.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_construct.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_deque.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_function.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_heap.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_pair.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_queue.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_threads.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_vector.h
../world_load..o: /usr/local/include/g++-v3/bits/stringfwd.h
../world_load..o: /usr/local/include/g++-v3/bits/type_traits.h
../world_load..o: /usr/local/include/g++-v3/cstddef
../world_load..o: /usr/local/include/g++-v3/cstdio
../world_load..o: /usr/local/include/g++-v3/exception
../world_load..o: /usr/local/include/g++-v3/exception_defines.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../world_load..o: /usr/local/include/g++-v3/new
../world_load..o: /usr/local/include/g++-v3/queue
../world_load..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../world_load..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../world_load..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../world_load..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../world..o: /home/vaughan/work/player/include/messages.h
../world..o: /home/vaughan/work/player/include/playercommon.h
../world..o: /home/vaughan/work/player/include/stage.h /usr/include/alloca.h
../world..o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
../world..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../world..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../world..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../world..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
../world..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
../world..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
../world..o: /usr/include/bits/ipc.h /usr/include/bits/locale.h
../world..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../world..o: /usr/include/bits/mathdef.h /usr/include/bits/mman.h
../world..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../world..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
../world..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../world..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../world..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
../world..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
../world..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
../world..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
../world..o: /usr/include/bits/socket.h /usr/include/bits/stat.h
../world..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../world..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../world..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../world..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../world..o: /usr/include/ctype.h /usr/include/endian.h /usr/include/errno.h
../world..o: /usr/include/fcntl.h /usr/include/features.h
../world..o: /usr/include/_G_config.h /usr/include/gconv.h
../world..o: /usr/include/getopt.h /usr/include/gnu/stubs.h
../world..o: /usr/include/iconv.h /usr/include/langinfo.h /usr/include/libio.h
../world..o: /usr/include/limits.h /usr/include/linux/errno.h
../world..o: /usr/include/linux/limits.h /usr/include/locale.h
../world..o: /usr/include/math.h /usr/include/netinet/in.h
../world..o: /usr/include/nl_types.h /usr/include/pthread.h /usr/include/pwd.h
../world..o: /usr/include/sched.h /usr/include/signal.h /usr/include/stdint.h
../world..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
../world..o: /usr/include/sys/cdefs.h /usr/include/sys/ipc.h
../world..o: /usr/include/sys/mman.h /usr/include/sys/select.h
../world..o: /usr/include/sys/sem.h /usr/include/sys/stat.h
../world..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../world..o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
../world..o: /usr/include/time.h /usr/include/ucontext.h /usr/include/unistd.h
../world..o: /usr/include/wchar.h /usr/include/wctype.h /usr/include/xlocale.h
../world..o: /usr/local/include/g++-v3/backward/backward_warning.h
../world..o: /usr/local/include/g++-v3/backward/iostream.h
../world..o: /usr/local/include/g++-v3/bits/basic_ios.h
../world..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../world..o: /usr/local/include/g++-v3/bits/basic_string.h
../world..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../world..o: /usr/local/include/g++-v3/bits/char_traits.h
../world..o: /usr/local/include/g++-v3/bits/codecvt.h
../world..o: /usr/local/include/g++-v3/bits/concept_check.h
../world..o: /usr/local/include/g++-v3/bits/fpos.h
../world..o: /usr/local/include/g++-v3/bits/functexcept.h
../world..o: /usr/local/include/g++-v3/bits/ios_base.h
../world..o: /usr/local/include/g++-v3/bits/istream.tcc
../world..o: /usr/local/include/g++-v3/bits/locale_facets.h
../world..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../world..o: /usr/local/include/g++-v3/bits/localefwd.h
../world..o: /usr/local/include/g++-v3/bits/ostream.tcc
../world..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../world..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../world..o: /usr/local/include/g++-v3/bits/std_cassert.h
../world..o: /usr/local/include/g++-v3/bits/std_cctype.h
../world..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../world..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../world..o: /usr/local/include/g++-v3/bits/std_climits.h
../world..o: /usr/local/include/g++-v3/bits/std_clocale.h
../world..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../world..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../world..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../world..o: /usr/local/include/g++-v3/bits/std_cstring.h
../world..o: /usr/local/include/g++-v3/bits/std_ctime.h
../world..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../world..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../world..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../world..o: /usr/local/include/g++-v3/bits/std_ios.h
../world..o: /usr/local/include/g++-v3/bits/std_iostream.h
../world..o: /usr/local/include/g++-v3/bits/std_istream.h
../world..o: /usr/local/include/g++-v3/bits/std_iterator.h
../world..o: /usr/local/include/g++-v3/bits/std_locale.h
../world..o: /usr/local/include/g++-v3/bits/std_memory.h
../world..o: /usr/local/include/g++-v3/bits/std_ostream.h
../world..o: /usr/local/include/g++-v3/bits/std_queue.h
../world..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../world..o: /usr/local/include/g++-v3/bits/std_string.h
../world..o: /usr/local/include/g++-v3/bits/std_vector.h
../world..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../world..o: /usr/local/include/g++-v3/bits/stl_algo.h
../world..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../world..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../world..o: /usr/local/include/g++-v3/bits/stl_construct.h
../world..o: /usr/local/include/g++-v3/bits/stl_deque.h
../world..o: /usr/local/include/g++-v3/bits/stl_function.h
../world..o: /usr/local/include/g++-v3/bits/stl_heap.h
../world..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../world..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../world..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../world..o: /usr/local/include/g++-v3/bits/stl_pair.h
../world..o: /usr/local/include/g++-v3/bits/stl_queue.h
../world..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../world..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../world..o: /usr/local/include/g++-v3/bits/stl_threads.h
../world..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../world..o: /usr/local/include/g++-v3/bits/stl_vector.h
../world..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../world..o: /usr/local/include/g++-v3/bits/stringfwd.h
../world..o: /usr/local/include/g++-v3/bits/type_traits.h
../world..o: /usr/local/include/g++-v3/cstddef
../world..o: /usr/local/include/g++-v3/cstdio
../world..o: /usr/local/include/g++-v3/exception
../world..o: /usr/local/include/g++-v3/exception_defines.h
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../world..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../world..o: /usr/local/include/g++-v3/typeinfo
../world..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../world..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../world..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../world..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../world..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../xs..o: /home/vaughan/work/player/include/messages.h
../xs..o: /home/vaughan/work/player/include/playercommon.h
../xs..o: /usr/include/alloca.h /usr/include/arpa/inet.h
../xs..o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
../xs..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../xs..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../xs..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../xs..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
../xs..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
../xs..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
../xs..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
../xs..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../xs..o: /usr/include/bits/nan.h /usr/include/bits/netdb.h
../xs..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../xs..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
../xs..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../xs..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
../xs..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
../xs..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
../xs..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
../xs..o: /usr/include/bits/socket.h /usr/include/bits/stat.h
../xs..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../xs..o: /usr/include/bits/types.h /usr/include/bits/uio.h
../xs..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
../xs..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../xs..o: /usr/include/bits/xopen_lim.h /usr/include/ctype.h
../xs..o: /usr/include/endian.h /usr/include/errno.h /usr/include/fcntl.h
../xs..o: /usr/include/features.h /usr/include/_G_config.h
../xs..o: /usr/include/gconv.h /usr/include/getopt.h /usr/include/gnu/stubs.h
../xs..o: /usr/include/iconv.h /usr/include/langinfo.h /usr/include/libio.h
../xs..o: /usr/include/limits.h /usr/include/linux/errno.h
../xs..o: /usr/include/linux/limits.h /usr/include/locale.h
../xs..o: /usr/include/math.h /usr/include/netdb.h /usr/include/netinet/in.h
../xs..o: /usr/include/nl_types.h /usr/include/pthread.h
../xs..o: /usr/include/rpc/netdb.h /usr/include/sched.h /usr/include/signal.h
../xs..o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
../xs..o: /usr/include/string.h /usr/include/sys/cdefs.h
../xs..o: /usr/include/sys/select.h /usr/include/sys/socket.h
../xs..o: /usr/include/sys/stat.h /usr/include/sys/sysmacros.h
../xs..o: /usr/include/sys/time.h /usr/include/sys/types.h
../xs..o: /usr/include/sys/ucontext.h /usr/include/sys/uio.h
../xs..o: /usr/include/time.h /usr/include/ucontext.h /usr/include/unistd.h
../xs..o: /usr/include/values.h /usr/include/wchar.h /usr/include/wctype.h
../xs..o: /usr/include/X11/keysymdef.h /usr/include/X11/keysym.h
../xs..o: /usr/include/X11/Xfuncproto.h /usr/include/X11/X.h
../xs..o: /usr/include/X11/Xutil.h /usr/include/xlocale.h
../xs..o: /usr/local/include/g++-v3/backward/backward_warning.h
../xs..o: /usr/local/include/g++-v3/backward/fstream.h
../xs..o: /usr/local/include/g++-v3/backward/iostream.h
../xs..o: /usr/local/include/g++-v3/bits/basic_file.h
../xs..o: /usr/local/include/g++-v3/bits/basic_ios.h
../xs..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../xs..o: /usr/local/include/g++-v3/bits/basic_string.h
../xs..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../xs..o: /usr/local/include/g++-v3/bits/char_traits.h
../xs..o: /usr/local/include/g++-v3/bits/codecvt.h
../xs..o: /usr/local/include/g++-v3/bits/concept_check.h
../xs..o: /usr/local/include/g++-v3/bits/fpos.h
../xs..o: /usr/local/include/g++-v3/bits/fstream.tcc
../xs..o: /usr/local/include/g++-v3/bits/functexcept.h
../xs..o: /usr/local/include/g++-v3/bits/ios_base.h
../xs..o: /usr/local/include/g++-v3/bits/istream.tcc
../xs..o: /usr/local/include/g++-v3/bits/locale_facets.h
../xs..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
../xs..o: /usr/local/include/g++-v3/bits/localefwd.h
../xs..o: /usr/local/include/g++-v3/bits/ostream.tcc
../xs..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
../xs..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../xs..o: /usr/local/include/g++-v3/bits/std_cassert.h
../xs..o: /usr/local/include/g++-v3/bits/std_cctype.h
../xs..o: /usr/local/include/g++-v3/bits/std_cerrno.h
../xs..o: /usr/local/include/g++-v3/bits/std_cfloat.h
../xs..o: /usr/local/include/g++-v3/bits/std_climits.h
../xs..o: /usr/local/include/g++-v3/bits/std_clocale.h
../xs..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../xs..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../xs..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../xs..o: /usr/local/include/g++-v3/bits/std_cstring.h
../xs..o: /usr/local/include/g++-v3/bits/std_ctime.h
../xs..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../xs..o: /usr/local/include/g++-v3/bits/std_cwctype.h
../xs..o: /usr/local/include/g++-v3/bits/std_fstream.h
../xs..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../xs..o: /usr/local/include/g++-v3/bits/std_ios.h
../xs..o: /usr/local/include/g++-v3/bits/std_iostream.h
../xs..o: /usr/local/include/g++-v3/bits/std_istream.h
../xs..o: /usr/local/include/g++-v3/bits/std_iterator.h
../xs..o: /usr/local/include/g++-v3/bits/std_locale.h
../xs..o: /usr/local/include/g++-v3/bits/std_map.h
../xs..o: /usr/local/include/g++-v3/bits/std_memory.h
../xs..o: /usr/local/include/g++-v3/bits/std_ostream.h
../xs..o: /usr/local/include/g++-v3/bits/std_queue.h
../xs..o: /usr/local/include/g++-v3/bits/std_streambuf.h
../xs..o: /usr/local/include/g++-v3/bits/std_string.h
../xs..o: /usr/local/include/g++-v3/bits/std_vector.h
../xs..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../xs..o: /usr/local/include/g++-v3/bits/stl_algo.h
../xs..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../xs..o: /usr/local/include/g++-v3/bits/stl_bvector.h
../xs..o: /usr/local/include/g++-v3/bits/stl_construct.h
../xs..o: /usr/local/include/g++-v3/bits/stl_deque.h
../xs..o: /usr/local/include/g++-v3/bits/stl_function.h
../xs..o: /usr/local/include/g++-v3/bits/stl_heap.h
../xs..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../xs..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../xs..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../xs..o: /usr/local/include/g++-v3/bits/stl_map.h
../xs..o: /usr/local/include/g++-v3/bits/stl_multimap.h
../xs..o: /usr/local/include/g++-v3/bits/stl_pair.h
../xs..o: /usr/local/include/g++-v3/bits/stl_queue.h
../xs..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../xs..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../xs..o: /usr/local/include/g++-v3/bits/stl_threads.h
../xs..o: /usr/local/include/g++-v3/bits/stl_tree.h
../xs..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../xs..o: /usr/local/include/g++-v3/bits/stl_vector.h
../xs..o: /usr/local/include/g++-v3/bits/streambuf.tcc
../xs..o: /usr/local/include/g++-v3/bits/stringfwd.h
../xs..o: /usr/local/include/g++-v3/bits/type_traits.h
../xs..o: /usr/local/include/g++-v3/cstddef /usr/local/include/g++-v3/cstdio
../xs..o: /usr/local/include/g++-v3/exception
../xs..o: /usr/local/include/g++-v3/exception_defines.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/basic_file_model.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
../xs..o: /usr/local/include/g++-v3/map /usr/local/include/g++-v3/new
../xs..o: /usr/local/include/g++-v3/queue /usr/local/include/g++-v3/typeinfo
../xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xarch.h
../xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xlib.h
../xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xosdefs.h
../xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xos.h
