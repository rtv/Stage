
##############################################################################
# include the main configurations from the top-level directory
#

include ../Makefile.common

##############################################################################
# Build section

# choose one of these that suits your build (debug/profile/production)

# production
#CCFLAGS = -O3 
# debug
CCFLAGS = -g3 
# debug + profile
#CCFLAGS = -pg -g3 

# Flags common to all targets
#
CCFLAGS += -Wall -DREENTRANT -I../include -I$(PLAYER_INC) $(ZLIB_FLAGS) 
CCFLAGS += -DVERSION=\"$(VERSION)\"

# production/debug link
LINKFLAGS = -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
# profile link
#LINKFLAGS = -pg -lm -lstdc++ -lpthread  $(ZLIB_LIBS)


##############################################################################
# Targets

all: stage rtkstage xs truthlog 

stage:
	cd stage_objs && make stage

rtkstage:
	cd rtkstage_objs && make rtkstage

# Target XS - the X Player/Stage Interface
#
LIBDIRS = -L../../client_libs/c++ -L$(INSTALL_PREFIX)/lib -L/usr/local/player/lib
LIBS = $(LIBDIRS) -lplayerclient


xs :CCFLAGS += -I/usr/X11R6/include 
xs: LINKFLAGS += -L/usr/X11R6/lib -lX11 -L$(PLAYER_LIB) -lplayerclient
xs : xs.cc render.cc
	$(CC) xs.cc render.cc $(CCFLAGS) $(LINKFLAGS) -o xs

truthlog: truthlog.cc 
	$(CC) -I../include -I$(PLAYER_INC) -lstdc++ truthlog.cc -o truthlog

clean:
	rm -f *.o *~ core xs truthlog 
	cd stage_objs && make clean
	cd rtkstage_objs && make clean

dep:
	$(MAKEDEP) -Y -I../include -I$(PLAYER_INC) -s "# Dependancies (make dep)" truthlog.cc xs.cc render.cc 2>/dev/null
	$(RM) -f Makefile.bak
	cd stage_objs && make dep
	cd rtkstage_objs && make dep

clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak
	cd stage_objs && make clean_dep
	cd rtkstage_objs && make clean_dep

%.o : %.cc
	${CC} ${CCFLAGS} -c $< -o $@

##############################################################################
# Dependancies (make dep)


render..o: ../include/stage_types.hh ../include/truthserver.hh
render..o: ../include/xs.hh /usr/include/alloca.h /usr/include/asm/errno.h
render..o: /usr/include/assert.h /usr/include/bits/confname.h
render..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
render..o: /usr/include/bits/errno.h /usr/include/bits/fcntl.h
render..o: /usr/include/bits/huge_val.h /usr/include/bits/locale.h
render..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
render..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
render..o: /usr/include/bits/poll.h /usr/include/bits/posix1_lim.h
render..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
render..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
render..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
render..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
render..o: /usr/include/bits/time.h /usr/include/bits/types.h
render..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
render..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
render..o: /usr/include/bits/xopen_lim.h /usr/include/ctype.h
render..o: /usr/include/endian.h /usr/include/errno.h /usr/include/fcntl.h
render..o: /usr/include/features.h /usr/include/_G_config.h
render..o: /usr/include/gconv.h /usr/include/getopt.h /usr/include/gnu/stubs.h
render..o: /usr/include/iconv.h /usr/include/langinfo.h /usr/include/libio.h
render..o: /usr/include/limits.h /usr/include/linux/errno.h
render..o: /usr/include/linux/limits.h /usr/include/locale.h
render..o: /usr/include/math.h /usr/include/nl_types.h /usr/include/stdint.h
render..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
render..o: /usr/include/sys/cdefs.h /usr/include/sys/poll.h
render..o: /usr/include/sys/select.h /usr/include/sys/stat.h
render..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
render..o: /usr/include/sys/types.h /usr/include/time.h /usr/include/unistd.h
render..o: /usr/include/wchar.h /usr/include/wctype.h
render..o: /usr/include/X11/keysymdef.h /usr/include/X11/keysym.h
render..o: /usr/include/X11/Xfuncproto.h /usr/include/X11/X.h
render..o: /usr/include/X11/Xutil.h /usr/include/xlocale.h
render..o: /usr/local/include/g++-v3/backward/backward_warning.h
render..o: /usr/local/include/g++-v3/backward/iostream.h
render..o: /usr/local/include/g++-v3/bits/basic_ios.h
render..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
render..o: /usr/local/include/g++-v3/bits/basic_string.h
render..o: /usr/local/include/g++-v3/bits/basic_string.tcc
render..o: /usr/local/include/g++-v3/bits/char_traits.h
render..o: /usr/local/include/g++-v3/bits/codecvt.h
render..o: /usr/local/include/g++-v3/bits/concept_check.h
render..o: /usr/local/include/g++-v3/bits/fpos.h
render..o: /usr/local/include/g++-v3/bits/functexcept.h
render..o: /usr/local/include/g++-v3/bits/ios_base.h
render..o: /usr/local/include/g++-v3/bits/istream.tcc
render..o: /usr/local/include/g++-v3/bits/locale_facets.h
render..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
render..o: /usr/local/include/g++-v3/bits/localefwd.h
render..o: /usr/local/include/g++-v3/bits/ostream.tcc
render..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
render..o: /usr/local/include/g++-v3/bits/std_algorithm.h
render..o: /usr/local/include/g++-v3/bits/std_cassert.h
render..o: /usr/local/include/g++-v3/bits/std_cctype.h
render..o: /usr/local/include/g++-v3/bits/std_cerrno.h
render..o: /usr/local/include/g++-v3/bits/std_cfloat.h
render..o: /usr/local/include/g++-v3/bits/std_climits.h
render..o: /usr/local/include/g++-v3/bits/std_clocale.h
render..o: /usr/local/include/g++-v3/bits/std_cstddef.h
render..o: /usr/local/include/g++-v3/bits/std_cstdio.h
render..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
render..o: /usr/local/include/g++-v3/bits/std_cstring.h
render..o: /usr/local/include/g++-v3/bits/std_ctime.h
render..o: /usr/local/include/g++-v3/bits/std_cwchar.h
render..o: /usr/local/include/g++-v3/bits/std_cwctype.h
render..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
render..o: /usr/local/include/g++-v3/bits/std_ios.h
render..o: /usr/local/include/g++-v3/bits/std_iostream.h
render..o: /usr/local/include/g++-v3/bits/std_istream.h
render..o: /usr/local/include/g++-v3/bits/std_iterator.h
render..o: /usr/local/include/g++-v3/bits/std_locale.h
render..o: /usr/local/include/g++-v3/bits/std_memory.h
render..o: /usr/local/include/g++-v3/bits/std_ostream.h
render..o: /usr/local/include/g++-v3/bits/std_streambuf.h
render..o: /usr/local/include/g++-v3/bits/std_string.h
render..o: /usr/local/include/g++-v3/bits/std_vector.h
render..o: /usr/local/include/g++-v3/bits/stl_algobase.h
render..o: /usr/local/include/g++-v3/bits/stl_algo.h
render..o: /usr/local/include/g++-v3/bits/stl_alloc.h
render..o: /usr/local/include/g++-v3/bits/stl_bvector.h
render..o: /usr/local/include/g++-v3/bits/stl_construct.h
render..o: /usr/local/include/g++-v3/bits/stl_heap.h
render..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
render..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
render..o: /usr/local/include/g++-v3/bits/stl_iterator.h
render..o: /usr/local/include/g++-v3/bits/stl_pair.h
render..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
render..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
render..o: /usr/local/include/g++-v3/bits/stl_threads.h
render..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
render..o: /usr/local/include/g++-v3/bits/stl_vector.h
render..o: /usr/local/include/g++-v3/bits/streambuf.tcc
render..o: /usr/local/include/g++-v3/bits/stringfwd.h
render..o: /usr/local/include/g++-v3/bits/type_traits.h
render..o: /usr/local/include/g++-v3/cstddef /usr/local/include/g++-v3/cstdio
render..o: /usr/local/include/g++-v3/exception
render..o: /usr/local/include/g++-v3/exception_defines.h
render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
render..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
render..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/typeinfo
render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xarch.h
render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xlib.h
render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xosdefs.h
render..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xos.h
render..o: /usr/local/player-1.0-gamma/include/bpsproxy.h
render..o: /usr/local/player-1.0-gamma/include/clientproxy.h
render..o: /usr/local/player-1.0-gamma/include/gpsproxy.h
render..o: /usr/local/player-1.0-gamma/include/gripperproxy.h
render..o: /usr/local/player-1.0-gamma/include/laserbeaconproxy.h
render..o: /usr/local/player-1.0-gamma/include/laserproxy.h
render..o: /usr/local/player-1.0-gamma/include/messages.h
render..o: /usr/local/player-1.0-gamma/include/miscproxy.h
render..o: /usr/local/player-1.0-gamma/include/occupancyproxy.h
render..o: /usr/local/player-1.0-gamma/include/playercclient.h
render..o: /usr/local/player-1.0-gamma/include/playerclient.h
render..o: /usr/local/player-1.0-gamma/include/playercommon.h
render..o: /usr/local/player-1.0-gamma/include/playermulticlient.h
render..o: /usr/local/player-1.0-gamma/include/positionproxy.h
render..o: /usr/local/player-1.0-gamma/include/ptzproxy.h
render..o: /usr/local/player-1.0-gamma/include/sonarproxy.h
render..o: /usr/local/player-1.0-gamma/include/speechproxy.h
render..o: /usr/local/player-1.0-gamma/include/truthproxy.h
render..o: /usr/local/player-1.0-gamma/include/visionproxy.h
truthlog..o: ../include/stage_types.hh ../include/truthserver.hh
truthlog..o: /usr/include/alloca.h /usr/include/arpa/inet.h
truthlog..o: /usr/include/asm/errno.h /usr/include/asm/sigcontext.h
truthlog..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
truthlog..o: /usr/include/assert.h /usr/include/bits/byteswap.h
truthlog..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
truthlog..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
truthlog..o: /usr/include/bits/fcntl.h /usr/include/bits/huge_val.h
truthlog..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
truthlog..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
truthlog..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
truthlog..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
truthlog..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
truthlog..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
truthlog..o: /usr/include/bits/select.h /usr/include/bits/sigaction.h
truthlog..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
truthlog..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
truthlog..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
truthlog..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
truthlog..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
truthlog..o: /usr/include/bits/time.h /usr/include/bits/types.h
truthlog..o: /usr/include/bits/uio.h /usr/include/bits/waitflags.h
truthlog..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
truthlog..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
truthlog..o: /usr/include/ctype.h /usr/include/endian.h /usr/include/errno.h
truthlog..o: /usr/include/fcntl.h /usr/include/features.h
truthlog..o: /usr/include/_G_config.h /usr/include/gconv.h
truthlog..o: /usr/include/getopt.h /usr/include/gnu/stubs.h
truthlog..o: /usr/include/iconv.h /usr/include/langinfo.h /usr/include/libio.h
truthlog..o: /usr/include/limits.h /usr/include/linux/errno.h
truthlog..o: /usr/include/linux/limits.h /usr/include/locale.h
truthlog..o: /usr/include/math.h /usr/include/netinet/in.h
truthlog..o: /usr/include/nl_types.h /usr/include/pthread.h
truthlog..o: /usr/include/sched.h /usr/include/signal.h /usr/include/stdint.h
truthlog..o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/string.h
truthlog..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
truthlog..o: /usr/include/sys/socket.h /usr/include/sys/stat.h
truthlog..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
truthlog..o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
truthlog..o: /usr/include/sys/uio.h /usr/include/time.h
truthlog..o: /usr/include/ucontext.h /usr/include/unistd.h
truthlog..o: /usr/include/values.h /usr/include/wchar.h /usr/include/wctype.h
truthlog..o: /usr/include/xlocale.h
truthlog..o: /usr/local/include/g++-v3/backward/backward_warning.h
truthlog..o: /usr/local/include/g++-v3/backward/iostream.h
truthlog..o: /usr/local/include/g++-v3/bits/basic_ios.h
truthlog..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
truthlog..o: /usr/local/include/g++-v3/bits/basic_string.h
truthlog..o: /usr/local/include/g++-v3/bits/basic_string.tcc
truthlog..o: /usr/local/include/g++-v3/bits/char_traits.h
truthlog..o: /usr/local/include/g++-v3/bits/codecvt.h
truthlog..o: /usr/local/include/g++-v3/bits/concept_check.h
truthlog..o: /usr/local/include/g++-v3/bits/fpos.h
truthlog..o: /usr/local/include/g++-v3/bits/functexcept.h
truthlog..o: /usr/local/include/g++-v3/bits/ios_base.h
truthlog..o: /usr/local/include/g++-v3/bits/istream.tcc
truthlog..o: /usr/local/include/g++-v3/bits/locale_facets.h
truthlog..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
truthlog..o: /usr/local/include/g++-v3/bits/localefwd.h
truthlog..o: /usr/local/include/g++-v3/bits/ostream.tcc
truthlog..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
truthlog..o: /usr/local/include/g++-v3/bits/std_algorithm.h
truthlog..o: /usr/local/include/g++-v3/bits/std_cassert.h
truthlog..o: /usr/local/include/g++-v3/bits/std_cctype.h
truthlog..o: /usr/local/include/g++-v3/bits/std_cerrno.h
truthlog..o: /usr/local/include/g++-v3/bits/std_cfloat.h
truthlog..o: /usr/local/include/g++-v3/bits/std_climits.h
truthlog..o: /usr/local/include/g++-v3/bits/std_clocale.h
truthlog..o: /usr/local/include/g++-v3/bits/std_cstddef.h
truthlog..o: /usr/local/include/g++-v3/bits/std_cstdio.h
truthlog..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
truthlog..o: /usr/local/include/g++-v3/bits/std_cstring.h
truthlog..o: /usr/local/include/g++-v3/bits/std_ctime.h
truthlog..o: /usr/local/include/g++-v3/bits/std_cwchar.h
truthlog..o: /usr/local/include/g++-v3/bits/std_cwctype.h
truthlog..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
truthlog..o: /usr/local/include/g++-v3/bits/std_ios.h
truthlog..o: /usr/local/include/g++-v3/bits/std_iostream.h
truthlog..o: /usr/local/include/g++-v3/bits/std_istream.h
truthlog..o: /usr/local/include/g++-v3/bits/std_iterator.h
truthlog..o: /usr/local/include/g++-v3/bits/std_locale.h
truthlog..o: /usr/local/include/g++-v3/bits/std_memory.h
truthlog..o: /usr/local/include/g++-v3/bits/std_ostream.h
truthlog..o: /usr/local/include/g++-v3/bits/std_streambuf.h
truthlog..o: /usr/local/include/g++-v3/bits/std_string.h
truthlog..o: /usr/local/include/g++-v3/bits/std_vector.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_algobase.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_algo.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_alloc.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_bvector.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_construct.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_heap.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_iterator.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_pair.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_threads.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
truthlog..o: /usr/local/include/g++-v3/bits/stl_vector.h
truthlog..o: /usr/local/include/g++-v3/bits/streambuf.tcc
truthlog..o: /usr/local/include/g++-v3/bits/stringfwd.h
truthlog..o: /usr/local/include/g++-v3/bits/type_traits.h
truthlog..o: /usr/local/include/g++-v3/cstddef
truthlog..o: /usr/local/include/g++-v3/cstdio
truthlog..o: /usr/local/include/g++-v3/exception
truthlog..o: /usr/local/include/g++-v3/exception_defines.h
truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
truthlog..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
truthlog..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/typeinfo
truthlog..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
truthlog..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
truthlog..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
truthlog..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
truthlog..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
truthlog..o: /usr/local/player-1.0-gamma/include/messages.h
truthlog..o: /usr/local/player-1.0-gamma/include/playercommon.h
xs..o: ../include/stage_types.hh ../include/truthserver.hh ../include/xs.hh
xs..o: /usr/include/alloca.h /usr/include/arpa/inet.h /usr/include/asm/errno.h
xs..o: /usr/include/asm/sigcontext.h /usr/include/asm/socket.h
xs..o: /usr/include/asm/sockios.h /usr/include/assert.h
xs..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
xs..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
xs..o: /usr/include/bits/errno.h /usr/include/bits/fcntl.h
xs..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
xs..o: /usr/include/bits/initspin.h /usr/include/bits/locale.h
xs..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
xs..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
xs..o: /usr/include/bits/netdb.h /usr/include/bits/poll.h
xs..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
xs..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
xs..o: /usr/include/bits/sched.h /usr/include/bits/select.h
xs..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
xs..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
xs..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
xs..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
xs..o: /usr/include/bits/socket.h /usr/include/bits/stat.h
xs..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
xs..o: /usr/include/bits/types.h /usr/include/bits/uio.h
xs..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
xs..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
xs..o: /usr/include/bits/xopen_lim.h /usr/include/ctype.h
xs..o: /usr/include/endian.h /usr/include/errno.h /usr/include/fcntl.h
xs..o: /usr/include/features.h /usr/include/_G_config.h /usr/include/gconv.h
xs..o: /usr/include/getopt.h /usr/include/gnu/stubs.h /usr/include/iconv.h
xs..o: /usr/include/langinfo.h /usr/include/libio.h /usr/include/limits.h
xs..o: /usr/include/linux/errno.h /usr/include/linux/limits.h
xs..o: /usr/include/locale.h /usr/include/math.h /usr/include/netdb.h
xs..o: /usr/include/netinet/in.h /usr/include/nl_types.h
xs..o: /usr/include/pthread.h /usr/include/rpc/netdb.h /usr/include/sched.h
xs..o: /usr/include/signal.h /usr/include/stdint.h /usr/include/stdio.h
xs..o: /usr/include/stdlib.h /usr/include/string.h /usr/include/sys/cdefs.h
xs..o: /usr/include/sys/poll.h /usr/include/sys/select.h
xs..o: /usr/include/sys/socket.h /usr/include/sys/stat.h
xs..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
xs..o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
xs..o: /usr/include/sys/uio.h /usr/include/time.h /usr/include/ucontext.h
xs..o: /usr/include/unistd.h /usr/include/values.h /usr/include/wchar.h
xs..o: /usr/include/wctype.h /usr/include/X11/keysymdef.h
xs..o: /usr/include/X11/keysym.h /usr/include/X11/Xfuncproto.h
xs..o: /usr/include/X11/X.h /usr/include/X11/Xutil.h /usr/include/xlocale.h
xs..o: /usr/local/include/g++-v3/backward/backward_warning.h
xs..o: /usr/local/include/g++-v3/backward/fstream.h
xs..o: /usr/local/include/g++-v3/backward/iostream.h
xs..o: /usr/local/include/g++-v3/bits/basic_file.h
xs..o: /usr/local/include/g++-v3/bits/basic_ios.h
xs..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
xs..o: /usr/local/include/g++-v3/bits/basic_string.h
xs..o: /usr/local/include/g++-v3/bits/basic_string.tcc
xs..o: /usr/local/include/g++-v3/bits/char_traits.h
xs..o: /usr/local/include/g++-v3/bits/codecvt.h
xs..o: /usr/local/include/g++-v3/bits/concept_check.h
xs..o: /usr/local/include/g++-v3/bits/fpos.h
xs..o: /usr/local/include/g++-v3/bits/fstream.tcc
xs..o: /usr/local/include/g++-v3/bits/functexcept.h
xs..o: /usr/local/include/g++-v3/bits/ios_base.h
xs..o: /usr/local/include/g++-v3/bits/istream.tcc
xs..o: /usr/local/include/g++-v3/bits/locale_facets.h
xs..o: /usr/local/include/g++-v3/bits/locale_facets.tcc
xs..o: /usr/local/include/g++-v3/bits/localefwd.h
xs..o: /usr/local/include/g++-v3/bits/ostream.tcc
xs..o: /usr/local/include/g++-v3/bits/sbuf_iter.h
xs..o: /usr/local/include/g++-v3/bits/std_algorithm.h
xs..o: /usr/local/include/g++-v3/bits/std_cassert.h
xs..o: /usr/local/include/g++-v3/bits/std_cctype.h
xs..o: /usr/local/include/g++-v3/bits/std_cerrno.h
xs..o: /usr/local/include/g++-v3/bits/std_cfloat.h
xs..o: /usr/local/include/g++-v3/bits/std_climits.h
xs..o: /usr/local/include/g++-v3/bits/std_clocale.h
xs..o: /usr/local/include/g++-v3/bits/std_cstddef.h
xs..o: /usr/local/include/g++-v3/bits/std_cstdio.h
xs..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
xs..o: /usr/local/include/g++-v3/bits/std_cstring.h
xs..o: /usr/local/include/g++-v3/bits/std_ctime.h
xs..o: /usr/local/include/g++-v3/bits/std_cwchar.h
xs..o: /usr/local/include/g++-v3/bits/std_cwctype.h
xs..o: /usr/local/include/g++-v3/bits/std_fstream.h
xs..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
xs..o: /usr/local/include/g++-v3/bits/std_ios.h
xs..o: /usr/local/include/g++-v3/bits/std_iostream.h
xs..o: /usr/local/include/g++-v3/bits/std_istream.h
xs..o: /usr/local/include/g++-v3/bits/std_iterator.h
xs..o: /usr/local/include/g++-v3/bits/std_locale.h
xs..o: /usr/local/include/g++-v3/bits/std_map.h
xs..o: /usr/local/include/g++-v3/bits/std_memory.h
xs..o: /usr/local/include/g++-v3/bits/std_ostream.h
xs..o: /usr/local/include/g++-v3/bits/std_queue.h
xs..o: /usr/local/include/g++-v3/bits/std_streambuf.h
xs..o: /usr/local/include/g++-v3/bits/std_string.h
xs..o: /usr/local/include/g++-v3/bits/std_vector.h
xs..o: /usr/local/include/g++-v3/bits/stl_algobase.h
xs..o: /usr/local/include/g++-v3/bits/stl_algo.h
xs..o: /usr/local/include/g++-v3/bits/stl_alloc.h
xs..o: /usr/local/include/g++-v3/bits/stl_bvector.h
xs..o: /usr/local/include/g++-v3/bits/stl_construct.h
xs..o: /usr/local/include/g++-v3/bits/stl_deque.h
xs..o: /usr/local/include/g++-v3/bits/stl_function.h
xs..o: /usr/local/include/g++-v3/bits/stl_heap.h
xs..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
xs..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
xs..o: /usr/local/include/g++-v3/bits/stl_iterator.h
xs..o: /usr/local/include/g++-v3/bits/stl_map.h
xs..o: /usr/local/include/g++-v3/bits/stl_multimap.h
xs..o: /usr/local/include/g++-v3/bits/stl_pair.h
xs..o: /usr/local/include/g++-v3/bits/stl_queue.h
xs..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
xs..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
xs..o: /usr/local/include/g++-v3/bits/stl_threads.h
xs..o: /usr/local/include/g++-v3/bits/stl_tree.h
xs..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
xs..o: /usr/local/include/g++-v3/bits/stl_vector.h
xs..o: /usr/local/include/g++-v3/bits/streambuf.tcc
xs..o: /usr/local/include/g++-v3/bits/stringfwd.h
xs..o: /usr/local/include/g++-v3/bits/type_traits.h
xs..o: /usr/local/include/g++-v3/cstddef /usr/local/include/g++-v3/cstdio
xs..o: /usr/local/include/g++-v3/exception
xs..o: /usr/local/include/g++-v3/exception_defines.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/basic_file_model.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++locale.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_base.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/ctype_inline.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
xs..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/std_limits.h
xs..o: /usr/local/include/g++-v3/map /usr/local/include/g++-v3/new
xs..o: /usr/local/include/g++-v3/queue /usr/local/include/g++-v3/typeinfo
xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xarch.h
xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xlib.h
xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xosdefs.h
xs..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/X11/Xos.h
xs..o: /usr/local/player-1.0-gamma/include/bpsproxy.h
xs..o: /usr/local/player-1.0-gamma/include/clientproxy.h
xs..o: /usr/local/player-1.0-gamma/include/gpsproxy.h
xs..o: /usr/local/player-1.0-gamma/include/gripperproxy.h
xs..o: /usr/local/player-1.0-gamma/include/laserbeaconproxy.h
xs..o: /usr/local/player-1.0-gamma/include/laserproxy.h
xs..o: /usr/local/player-1.0-gamma/include/messages.h
xs..o: /usr/local/player-1.0-gamma/include/miscproxy.h
xs..o: /usr/local/player-1.0-gamma/include/occupancyproxy.h
xs..o: /usr/local/player-1.0-gamma/include/playercclient.h
xs..o: /usr/local/player-1.0-gamma/include/playerclient.h
xs..o: /usr/local/player-1.0-gamma/include/playercommon.h
xs..o: /usr/local/player-1.0-gamma/include/playermulticlient.h
xs..o: /usr/local/player-1.0-gamma/include/positionproxy.h
xs..o: /usr/local/player-1.0-gamma/include/ptzproxy.h
xs..o: /usr/local/player-1.0-gamma/include/sonarproxy.h
xs..o: /usr/local/player-1.0-gamma/include/speechproxy.h
xs..o: /usr/local/player-1.0-gamma/include/truthproxy.h
xs..o: /usr/local/player-1.0-gamma/include/visionproxy.h
