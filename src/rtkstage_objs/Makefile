
##############################################################################
# include the main configurations from the top-level directory
#

include ../../Makefile.common

#############################################################################
# These are the key paths needed for the build

RTK_DIR = ../../rtk
# rtkstage needs RTK installed and these paths set correctly
# RTK headers
RTK_INC = $(RTK_DIR)/include
# RTK libraries
RTK_LIB = $(RTK_DIR)/lib

##############################################################################
# Build section

# Objects common to all targets
#
OBJS =  world.o world_load.o truthserver.o envserver.o image.o \
	entity.o entityfactory.o playerdevice.o \
	positiondevice.o laserdevice.o sonardevice.o miscdevice.o \
	ptzdevice.o boxobstacle.o \
	laserbeacon.o laserbeacondevice.o visiondevice.o visionbeacon.o \
	broadcastdevice.o puck.o gpsdevice.o gripperdevice.o rtk_main.o \
	matrix.o raytrace.o

# choose one of these that suits your build (debug/profile/production)

# debug
CCFLAGS = -g3 
# debug + profile
#CCFLAGS = -pg -g3 
# production
#CCFLAGS = -O3 

# Flags common to all targets
#
CCFLAGS += -Wall -DREENTRANT -I../../include -I$(PLAYER_INC) $(ZLIB_FLAGS) 
CCFLAGS += -DVERSION=\"$(VERSION)\"
CCFLAGS += -DINCLUDE_RTK -I$(RTK_INC) `gtk-config --cflags`
CCFLAGS += -DCOLOR_DATABASE=\"$(COLOR_DATABASE)\"

# production/debug link
LINKFLAGS = -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
# profile link
#LINKFLAGS = -pg -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
LINKFLAGS += -L$(RTK_LIB) -lrtk `gtk-config --libs gtk gthread`

# Standard tools
#
CC := g++
LINKER = g++
MAKEDEP = makedepend

.SUFFIXES: .cc .o

##############################################################################
# Targets

all: rtkstage 

# Target stage with RTK GUI
#
rtkstage : $(OBJS) rtk
	$(LINKER) $(OBJS) $(LINKFLAGS) -o $@

rtk:
	cd $(RTK_DIR) && make

clean:
	rm -f *.o *~ core rtkstage

dep:
	$(MAKEDEP) -Y -- $(CCFLAGS) -- -s "# Dependancies (make dep)" ../*.cc 2>/dev/null
	$(RM) -f Makefile.bak

clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak

%.o : ../%.cc
	$(CC) $(CCFLAGS) -c $< -o $@

##############################################################################
# Dependancies (make dep)


../boxobstacle..o: /usr/include/alloca.h /usr/include/asm/socket.h
../boxobstacle..o: /usr/include/asm/sockios.h /usr/include/assert.h
../boxobstacle..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../boxobstacle..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../boxobstacle..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../boxobstacle..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../boxobstacle..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../boxobstacle..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../boxobstacle..o: /usr/include/bits/posix1_lim.h
../boxobstacle..o: /usr/include/bits/posix2_lim.h
../boxobstacle..o: /usr/include/bits/posix_opt.h
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
../boxobstacle..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../boxobstacle..o: /usr/include/gnu/stubs.h
../boxobstacle..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../boxobstacle..o: /usr/include/gtk-1.2/gdk/gdk.h
../boxobstacle..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../boxobstacle..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtk.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtklist.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktable.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktext.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktree.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../boxobstacle..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../boxobstacle..o: /usr/include/limits.h /usr/include/linux/limits.h
../boxobstacle..o: /usr/include/math.h /usr/include/netinet/in.h
../boxobstacle..o: /usr/include/pthread.h /usr/include/sched.h
../boxobstacle..o: /usr/include/signal.h /usr/include/stdint.h
../boxobstacle..o: /usr/include/stdio.h /usr/include/stdlib.h
../boxobstacle..o: /usr/include/string.h /usr/include/sys/cdefs.h
../boxobstacle..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../boxobstacle..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../boxobstacle..o: /usr/include/sys/time.h /usr/include/sys/types.h
../boxobstacle..o: /usr/include/time.h /usr/include/unistd.h
../boxobstacle..o: /usr/include/wchar.h /usr/include/xlocale.h
../boxobstacle..o: /usr/lib/glib/include/glibconfig.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/basic_string.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../boxobstacle..o: /usr/local/include/g++-v3/bits/char_traits.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/concept_check.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/fpos.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/functexcept.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cassert.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_climits.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cstring.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_iterator.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_memory.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_queue.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/std_string.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_threads.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stl_vector.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/stringfwd.h
../boxobstacle..o: /usr/local/include/g++-v3/bits/type_traits.h
../boxobstacle..o: /usr/local/include/g++-v3/cstddef
../boxobstacle..o: /usr/local/include/g++-v3/cstdio
../boxobstacle..o: /usr/local/include/g++-v3/exception
../boxobstacle..o: /usr/local/include/g++-v3/exception_defines.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../boxobstacle..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../boxobstacle..o: /usr/local/include/g++-v3/new
../boxobstacle..o: /usr/local/include/g++-v3/queue
../boxobstacle..o: /usr/local/include/g++-v3/string
../boxobstacle..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../boxobstacle..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../boxobstacle..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../boxobstacle..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../boxobstacle..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../boxobstacle..o: /usr/local/player-1.0-gamma/include/messages.h
../boxobstacle..o: /usr/local/player-1.0-gamma/include/playercommon.h
../boxobstacle..o: /usr/local/player-1.0-gamma/include/stage.h
../broadcastdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../broadcastdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../broadcastdevice..o: /usr/include/bits/byteswap.h
../broadcastdevice..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../broadcastdevice..o: /usr/include/bits/environments.h
../broadcastdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../broadcastdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../broadcastdevice..o: /usr/include/bits/local_lim.h
../broadcastdevice..o: /usr/include/bits/mathcalls.h
../broadcastdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../broadcastdevice..o: /usr/include/bits/posix1_lim.h
../broadcastdevice..o: /usr/include/bits/posix2_lim.h
../broadcastdevice..o: /usr/include/bits/posix_opt.h
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
../broadcastdevice..o: /usr/include/gconv.h /usr/include/getopt.h
../broadcastdevice..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../broadcastdevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../broadcastdevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../broadcastdevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../broadcastdevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../broadcastdevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h
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
../broadcastdevice..o: /usr/include/unistd.h /usr/include/wchar.h
../broadcastdevice..o: /usr/include/xlocale.h
../broadcastdevice..o: /usr/lib/glib/include/glibconfig.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../broadcastdevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/std_string.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../broadcastdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../broadcastdevice..o: /usr/local/include/g++-v3/cstddef
../broadcastdevice..o: /usr/local/include/g++-v3/cstdio
../broadcastdevice..o: /usr/local/include/g++-v3/exception
../broadcastdevice..o: /usr/local/include/g++-v3/exception_defines.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../broadcastdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../broadcastdevice..o: /usr/local/include/g++-v3/new
../broadcastdevice..o: /usr/local/include/g++-v3/queue
../broadcastdevice..o: /usr/local/include/g++-v3/string
../broadcastdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../broadcastdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../broadcastdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../broadcastdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../broadcastdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../broadcastdevice..o: /usr/local/player-1.0-gamma/include/messages.h
../broadcastdevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../broadcastdevice..o: /usr/local/player-1.0-gamma/include/stage.h
../entityfactory..o: /usr/include/alloca.h /usr/include/assert.h
../entityfactory..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../entityfactory..o: /usr/include/bits/environments.h
../entityfactory..o: /usr/include/bits/huge_val.h /usr/include/bits/initspin.h
../entityfactory..o: /usr/include/bits/local_lim.h
../entityfactory..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../entityfactory..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../entityfactory..o: /usr/include/bits/posix2_lim.h
../entityfactory..o: /usr/include/bits/posix_opt.h
../entityfactory..o: /usr/include/bits/pthreadtypes.h
../entityfactory..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../entityfactory..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../entityfactory..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../entityfactory..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../entityfactory..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../entityfactory..o: /usr/include/bits/wordsize.h
../entityfactory..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../entityfactory..o: /usr/include/features.h /usr/include/_G_config.h
../entityfactory..o: /usr/include/gconv.h /usr/include/getopt.h
../entityfactory..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../entityfactory..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../entityfactory..o: /usr/include/gtk-1.2/gdk/gdk.h
../entityfactory..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../entityfactory..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtk.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtklist.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktable.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktext.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktree.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../entityfactory..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../entityfactory..o: /usr/include/limits.h /usr/include/linux/limits.h
../entityfactory..o: /usr/include/math.h /usr/include/pthread.h
../entityfactory..o: /usr/include/sched.h /usr/include/signal.h
../entityfactory..o: /usr/include/stdint.h /usr/include/stdio.h
../entityfactory..o: /usr/include/stdlib.h /usr/include/string.h
../entityfactory..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
../entityfactory..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../entityfactory..o: /usr/include/sys/types.h /usr/include/time.h
../entityfactory..o: /usr/include/unistd.h /usr/include/wchar.h
../entityfactory..o: /usr/include/xlocale.h /usr/lib/glib/include/glibconfig.h
../entityfactory..o: /usr/local/include/g++-v3/backward/backward_warning.h
../entityfactory..o: /usr/local/include/g++-v3/backward/slist.h
../entityfactory..o: /usr/local/include/g++-v3/bits/basic_string.h
../entityfactory..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../entityfactory..o: /usr/local/include/g++-v3/bits/char_traits.h
../entityfactory..o: /usr/local/include/g++-v3/bits/concept_check.h
../entityfactory..o: /usr/local/include/g++-v3/bits/fpos.h
../entityfactory..o: /usr/local/include/g++-v3/bits/functexcept.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cassert.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_climits.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cstring.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_iterator.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_memory.h
../entityfactory..o: /usr/local/include/g++-v3/bits/std_string.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_algo.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_construct.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_heap.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_pair.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_threads.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../entityfactory..o: /usr/local/include/g++-v3/bits/stringfwd.h
../entityfactory..o: /usr/local/include/g++-v3/bits/type_traits.h
../entityfactory..o: /usr/local/include/g++-v3/cstddef
../entityfactory..o: /usr/local/include/g++-v3/cstdio
../entityfactory..o: /usr/local/include/g++-v3/exception
../entityfactory..o: /usr/local/include/g++-v3/exception_defines.h
../entityfactory..o: /usr/local/include/g++-v3/ext/slist
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../entityfactory..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../entityfactory..o: /usr/local/include/g++-v3/new
../entityfactory..o: /usr/local/include/g++-v3/string
../entityfactory..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../entityfactory..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../entityfactory..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../entityfactory..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../entityfactory..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../entityfactory..o: /usr/local/player-1.0-gamma/include/messages.h
../entityfactory..o: /usr/local/player-1.0-gamma/include/playercommon.h
../entityfactory..o: /usr/local/player-1.0-gamma/include/stage.h
../entity..o: /usr/include/alloca.h /usr/include/asm/errno.h
../entity..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../entity..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../entity..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../entity..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
../entity..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../entity..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../entity..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
../entity..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../entity..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../entity..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
../entity..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../entity..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../entity..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../entity..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../entity..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../entity..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../entity..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../entity..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../entity..o: /usr/include/ctype.h /usr/include/endian.h /usr/include/errno.h
../entity..o: /usr/include/features.h /usr/include/_G_config.h
../entity..o: /usr/include/gconv.h /usr/include/getopt.h
../entity..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../entity..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../entity..o: /usr/include/gtk-1.2/gdk/gdk.h /usr/include/gtk-1.2/gdk/gdkrgb.h
../entity..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkgc.h /usr/include/gtk-1.2/gtk/gtk.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../entity..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../entity..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../entity..o: /usr/include/gtk-1.2/gtk/gtklist.h
../entity..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../entity..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../entity..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../entity..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktable.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktext.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktree.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../entity..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../entity..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/iconv.h
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
../entity..o: /usr/lib/glib/include/glibconfig.h
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
../entity..o: /usr/local/include/g++-v3/string
../entity..o: /usr/local/include/g++-v3/typeinfo
../entity..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../entity..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../entity..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../entity..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../entity..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../entity..o: /usr/local/player-1.0-gamma/include/messages.h
../entity..o: /usr/local/player-1.0-gamma/include/playercommon.h
../entity..o: /usr/local/player-1.0-gamma/include/stage.h
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
../envserver..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../envserver..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../envserver..o: /usr/include/gtk-1.2/gdk/gdk.h
../envserver..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../envserver..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtk.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtklist.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktable.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktext.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktree.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../envserver..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/iconv.h
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
../envserver..o: /usr/include/xlocale.h /usr/lib/glib/include/glibconfig.h
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
../envserver..o: /usr/local/include/g++-v3/string
../envserver..o: /usr/local/include/g++-v3/typeinfo
../envserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../envserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../envserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../envserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../envserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../envserver..o: /usr/local/player-1.0-gamma/include/messages.h
../envserver..o: /usr/local/player-1.0-gamma/include/playercommon.h
../envserver..o: /usr/local/player-1.0-gamma/include/stage.h
../gpsdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../gpsdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../gpsdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../gpsdevice..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../gpsdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../gpsdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../gpsdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../gpsdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../gpsdevice..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../gpsdevice..o: /usr/include/bits/posix_opt.h
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
../gpsdevice..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../gpsdevice..o: /usr/include/gnu/stubs.h
../gpsdevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../gpsdevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../gpsdevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../gpsdevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../gpsdevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../gpsdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../gpsdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../gpsdevice..o: /usr/include/pthread.h /usr/include/sched.h
../gpsdevice..o: /usr/include/signal.h /usr/include/stdint.h
../gpsdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../gpsdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../gpsdevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../gpsdevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../gpsdevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../gpsdevice..o: /usr/include/time.h /usr/include/unistd.h
../gpsdevice..o: /usr/include/wchar.h /usr/include/xlocale.h
../gpsdevice..o: /usr/lib/glib/include/glibconfig.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../gpsdevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/std_string.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../gpsdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../gpsdevice..o: /usr/local/include/g++-v3/cstddef
../gpsdevice..o: /usr/local/include/g++-v3/cstdio
../gpsdevice..o: /usr/local/include/g++-v3/exception
../gpsdevice..o: /usr/local/include/g++-v3/exception_defines.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../gpsdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../gpsdevice..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../gpsdevice..o: /usr/local/include/g++-v3/string
../gpsdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../gpsdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../gpsdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../gpsdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../gpsdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../gpsdevice..o: /usr/local/player-1.0-gamma/include/messages.h
../gpsdevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../gpsdevice..o: /usr/local/player-1.0-gamma/include/stage.h
../gripperdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../gripperdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../gripperdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../gripperdevice..o: /usr/include/bits/endian.h
../gripperdevice..o: /usr/include/bits/environments.h
../gripperdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../gripperdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../gripperdevice..o: /usr/include/bits/local_lim.h
../gripperdevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../gripperdevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../gripperdevice..o: /usr/include/bits/posix2_lim.h
../gripperdevice..o: /usr/include/bits/posix_opt.h
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
../gripperdevice..o: /usr/include/gconv.h /usr/include/getopt.h
../gripperdevice..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../gripperdevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../gripperdevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../gripperdevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../gripperdevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../gripperdevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../gripperdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../gripperdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../gripperdevice..o: /usr/include/pthread.h /usr/include/sched.h
../gripperdevice..o: /usr/include/signal.h /usr/include/stdint.h
../gripperdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../gripperdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../gripperdevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../gripperdevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../gripperdevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../gripperdevice..o: /usr/include/time.h /usr/include/unistd.h
../gripperdevice..o: /usr/include/values.h /usr/include/wchar.h
../gripperdevice..o: /usr/include/xlocale.h /usr/lib/glib/include/glibconfig.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../gripperdevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/std_string.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../gripperdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../gripperdevice..o: /usr/local/include/g++-v3/cstddef
../gripperdevice..o: /usr/local/include/g++-v3/cstdio
../gripperdevice..o: /usr/local/include/g++-v3/exception
../gripperdevice..o: /usr/local/include/g++-v3/exception_defines.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../gripperdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../gripperdevice..o: /usr/local/include/g++-v3/new
../gripperdevice..o: /usr/local/include/g++-v3/queue
../gripperdevice..o: /usr/local/include/g++-v3/string
../gripperdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../gripperdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../gripperdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../gripperdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../gripperdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../gripperdevice..o: /usr/local/player-1.0-gamma/include/messages.h
../gripperdevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../gripperdevice..o: /usr/local/player-1.0-gamma/include/stage.h
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
../laserbeacondevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../laserbeacondevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../laserbeacondevice..o: /usr/include/bits/byteswap.h
../laserbeacondevice..o: /usr/include/bits/confname.h
../laserbeacondevice..o: /usr/include/bits/endian.h
../laserbeacondevice..o: /usr/include/bits/environments.h
../laserbeacondevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../laserbeacondevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../laserbeacondevice..o: /usr/include/bits/local_lim.h
../laserbeacondevice..o: /usr/include/bits/mathcalls.h
../laserbeacondevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../laserbeacondevice..o: /usr/include/bits/posix1_lim.h
../laserbeacondevice..o: /usr/include/bits/posix2_lim.h
../laserbeacondevice..o: /usr/include/bits/posix_opt.h
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
../laserbeacondevice..o: /usr/include/gconv.h /usr/include/getopt.h
../laserbeacondevice..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../laserbeacondevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h
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
../laserbeacondevice..o: /usr/include/unistd.h /usr/include/wchar.h
../laserbeacondevice..o: /usr/include/xlocale.h
../laserbeacondevice..o: /usr/lib/glib/include/glibconfig.h
../laserbeacondevice..o: /usr/local/include/g++-v3/backward/backward_warning.h
../laserbeacondevice..o: /usr/local/include/g++-v3/backward/slist.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/fpos.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/std_string.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../laserbeacondevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
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
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../laserbeacondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../laserbeacondevice..o: /usr/local/include/g++-v3/new
../laserbeacondevice..o: /usr/local/include/g++-v3/queue
../laserbeacondevice..o: /usr/local/include/g++-v3/string
../laserbeacondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../laserbeacondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../laserbeacondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../laserbeacondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../laserbeacondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../laserbeacondevice..o: /usr/local/player-1.0-gamma/include/messages.h
../laserbeacondevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../laserbeacondevice..o: /usr/local/player-1.0-gamma/include/stage.h
../laserbeacon..o: /usr/include/alloca.h /usr/include/asm/socket.h
../laserbeacon..o: /usr/include/asm/sockios.h /usr/include/assert.h
../laserbeacon..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../laserbeacon..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../laserbeacon..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../laserbeacon..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../laserbeacon..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../laserbeacon..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../laserbeacon..o: /usr/include/bits/posix1_lim.h
../laserbeacon..o: /usr/include/bits/posix2_lim.h
../laserbeacon..o: /usr/include/bits/posix_opt.h
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
../laserbeacon..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../laserbeacon..o: /usr/include/gnu/stubs.h
../laserbeacon..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../laserbeacon..o: /usr/include/gtk-1.2/gdk/gdk.h
../laserbeacon..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../laserbeacon..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtk.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtklist.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktable.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktext.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktree.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../laserbeacon..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../laserbeacon..o: /usr/include/limits.h /usr/include/linux/limits.h
../laserbeacon..o: /usr/include/math.h /usr/include/netinet/in.h
../laserbeacon..o: /usr/include/pthread.h /usr/include/sched.h
../laserbeacon..o: /usr/include/signal.h /usr/include/stdint.h
../laserbeacon..o: /usr/include/stdio.h /usr/include/stdlib.h
../laserbeacon..o: /usr/include/string.h /usr/include/sys/cdefs.h
../laserbeacon..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../laserbeacon..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../laserbeacon..o: /usr/include/sys/time.h /usr/include/sys/types.h
../laserbeacon..o: /usr/include/time.h /usr/include/unistd.h
../laserbeacon..o: /usr/include/wchar.h /usr/include/xlocale.h
../laserbeacon..o: /usr/lib/glib/include/glibconfig.h
../laserbeacon..o: /usr/local/include/g++-v3/backward/backward_warning.h
../laserbeacon..o: /usr/local/include/g++-v3/backward/slist.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/basic_string.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../laserbeacon..o: /usr/local/include/g++-v3/bits/char_traits.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/concept_check.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/fpos.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/functexcept.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cassert.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_climits.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cstring.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_iterator.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_memory.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_queue.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/std_string.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../laserbeacon..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
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
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../laserbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../laserbeacon..o: /usr/local/include/g++-v3/new
../laserbeacon..o: /usr/local/include/g++-v3/queue
../laserbeacon..o: /usr/local/include/g++-v3/string
../laserbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../laserbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../laserbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../laserbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../laserbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../laserbeacon..o: /usr/local/player-1.0-gamma/include/messages.h
../laserbeacon..o: /usr/local/player-1.0-gamma/include/playercommon.h
../laserbeacon..o: /usr/local/player-1.0-gamma/include/stage.h
../laserdevice..o: /usr/include/alloca.h /usr/include/asm/errno.h
../laserdevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../laserdevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../laserdevice..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../laserdevice..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
../laserdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../laserdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../laserdevice..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
../laserdevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../laserdevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../laserdevice..o: /usr/include/bits/posix2_lim.h
../laserdevice..o: /usr/include/bits/posix_opt.h
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
../laserdevice..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../laserdevice..o: /usr/include/gnu/stubs.h
../laserdevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../laserdevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../laserdevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../laserdevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../laserdevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/iconv.h
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
../laserdevice..o: /usr/include/unistd.h /usr/include/wchar.h
../laserdevice..o: /usr/include/wctype.h /usr/include/xlocale.h
../laserdevice..o: /usr/lib/glib/include/glibconfig.h
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
../laserdevice..o: /usr/local/include/g++-v3/string
../laserdevice..o: /usr/local/include/g++-v3/typeinfo
../laserdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../laserdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../laserdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../laserdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../laserdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../laserdevice..o: /usr/local/player-1.0-gamma/include/messages.h
../laserdevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../laserdevice..o: /usr/local/player-1.0-gamma/include/stage.h
../main..o: /usr/include/alloca.h /usr/include/asm/sigcontext.h
../main..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../main..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../main..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../main..o: /usr/include/bits/environments.h /usr/include/bits/huge_val.h
../main..o: /usr/include/bits/in.h /usr/include/bits/initspin.h
../main..o: /usr/include/bits/ipc.h /usr/include/bits/local_lim.h
../main..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../main..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../main..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
../main..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../main..o: /usr/include/bits/select.h /usr/include/bits/sem.h
../main..o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
../main..o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
../main..o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
../main..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
../main..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
../main..o: /usr/include/bits/time.h /usr/include/bits/types.h
../main..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
../main..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../main..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../main..o: /usr/include/features.h /usr/include/_G_config.h
../main..o: /usr/include/gconv.h /usr/include/getopt.h
../main..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../main..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../main..o: /usr/include/gtk-1.2/gdk/gdk.h /usr/include/gtk-1.2/gdk/gdkrgb.h
../main..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../main..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../main..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../main..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../main..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../main..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../main..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../main..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../main..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../main..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../main..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../main..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../main..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../main..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../main..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../main..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../main..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../main..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../main..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../main..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../main..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../main..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../main..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../main..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../main..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../main..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../main..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../main..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../main..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../main..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../main..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../main..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../main..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../main..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../main..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../main..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../main..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../main..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../main..o: /usr/include/gtk-1.2/gtk/gtkgc.h /usr/include/gtk-1.2/gtk/gtk.h
../main..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../main..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../main..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../main..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../main..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../main..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../main..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../main..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../main..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../main..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../main..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../main..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../main..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../main..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../main..o: /usr/include/gtk-1.2/gtk/gtklist.h
../main..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../main..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../main..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../main..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../main..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../main..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../main..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../main..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../main..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../main..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../main..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../main..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../main..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../main..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../main..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../main..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../main..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../main..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../main..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../main..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../main..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../main..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../main..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../main..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../main..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../main..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../main..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../main..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../main..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../main..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../main..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../main..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../main..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../main..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../main..o: /usr/include/gtk-1.2/gtk/gtktable.h
../main..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../main..o: /usr/include/gtk-1.2/gtk/gtktext.h
../main..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../main..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../main..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../main..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../main..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../main..o: /usr/include/gtk-1.2/gtk/gtktree.h
../main..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../main..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../main..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../main..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../main..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../main..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../main..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../main..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../main..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../main..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../main..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../main..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../main..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../main..o: /usr/include/limits.h /usr/include/linux/limits.h
../main..o: /usr/include/math.h /usr/include/netinet/in.h
../main..o: /usr/include/pthread.h /usr/include/sched.h /usr/include/signal.h
../main..o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
../main..o: /usr/include/string.h /usr/include/sys/cdefs.h
../main..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../main..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../main..o: /usr/include/sys/time.h /usr/include/sys/types.h
../main..o: /usr/include/sys/ucontext.h /usr/include/time.h
../main..o: /usr/include/ucontext.h /usr/include/unistd.h /usr/include/wchar.h
../main..o: /usr/include/xlocale.h /usr/lib/glib/include/glibconfig.h
../main..o: /usr/local/include/g++-v3/bits/basic_string.h
../main..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../main..o: /usr/local/include/g++-v3/bits/char_traits.h
../main..o: /usr/local/include/g++-v3/bits/concept_check.h
../main..o: /usr/local/include/g++-v3/bits/fpos.h
../main..o: /usr/local/include/g++-v3/bits/functexcept.h
../main..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../main..o: /usr/local/include/g++-v3/bits/std_cassert.h
../main..o: /usr/local/include/g++-v3/bits/std_climits.h
../main..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../main..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../main..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../main..o: /usr/local/include/g++-v3/bits/std_cstring.h
../main..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../main..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../main..o: /usr/local/include/g++-v3/bits/std_iterator.h
../main..o: /usr/local/include/g++-v3/bits/std_memory.h
../main..o: /usr/local/include/g++-v3/bits/std_queue.h
../main..o: /usr/local/include/g++-v3/bits/std_string.h
../main..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../main..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../main..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../main..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../main..o: /usr/local/include/g++-v3/bits/stl_threads.h
../main..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../main..o: /usr/local/include/g++-v3/bits/stl_vector.h
../main..o: /usr/local/include/g++-v3/bits/stringfwd.h
../main..o: /usr/local/include/g++-v3/bits/type_traits.h
../main..o: /usr/local/include/g++-v3/cstddef /usr/local/include/g++-v3/cstdio
../main..o: /usr/local/include/g++-v3/exception
../main..o: /usr/local/include/g++-v3/exception_defines.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../main..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../main..o: /usr/local/include/g++-v3/string
../main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../main..o: /usr/local/player-1.0-gamma/include/messages.h
../main..o: /usr/local/player-1.0-gamma/include/playercommon.h
../main..o: /usr/local/player-1.0-gamma/include/stage.h
../matrix..o: /usr/include/alloca.h /usr/include/asm/errno.h
../matrix..o: /usr/include/assert.h /usr/include/bits/confname.h
../matrix..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../matrix..o: /usr/include/bits/errno.h /usr/include/bits/huge_val.h
../matrix..o: /usr/include/bits/initspin.h /usr/include/bits/locale.h
../matrix..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../matrix..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../matrix..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../matrix..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
../matrix..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../matrix..o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
../matrix..o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
../matrix..o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
../matrix..o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
../matrix..o: /usr/include/bits/wordsize.h /usr/include/bits/xopen_lim.h
../matrix..o: /usr/include/ctype.h /usr/include/endian.h /usr/include/errno.h
../matrix..o: /usr/include/features.h /usr/include/_G_config.h
../matrix..o: /usr/include/gconv.h /usr/include/getopt.h
../matrix..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../matrix..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../matrix..o: /usr/include/gtk-1.2/gdk/gdk.h /usr/include/gtk-1.2/gdk/gdkrgb.h
../matrix..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkgc.h /usr/include/gtk-1.2/gtk/gtk.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtklist.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktable.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktext.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktree.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../matrix..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/iconv.h
../matrix..o: /usr/include/langinfo.h /usr/include/libio.h
../matrix..o: /usr/include/limits.h /usr/include/linux/errno.h
../matrix..o: /usr/include/linux/limits.h /usr/include/locale.h
../matrix..o: /usr/include/math.h /usr/include/nl_types.h
../matrix..o: /usr/include/pthread.h /usr/include/sched.h
../matrix..o: /usr/include/signal.h /usr/include/stdint.h /usr/include/stdio.h
../matrix..o: /usr/include/stdlib.h /usr/include/string.h
../matrix..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
../matrix..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../matrix..o: /usr/include/sys/types.h /usr/include/time.h
../matrix..o: /usr/include/unistd.h /usr/include/wchar.h /usr/include/wctype.h
../matrix..o: /usr/include/xlocale.h /usr/include/zconf.h /usr/include/zlib.h
../matrix..o: /usr/lib/glib/include/glibconfig.h
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
../matrix..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/string
../matrix..o: /usr/local/include/g++-v3/typeinfo
../matrix..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../matrix..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../matrix..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../matrix..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../matrix..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../matrix..o: /usr/local/player-1.0-gamma/include/messages.h
../matrix..o: /usr/local/player-1.0-gamma/include/playercommon.h
../matrix..o: /usr/local/player-1.0-gamma/include/stage.h
../miscdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../miscdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../miscdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../miscdevice..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../miscdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../miscdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../miscdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../miscdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../miscdevice..o: /usr/include/bits/posix1_lim.h
../miscdevice..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
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
../miscdevice..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../miscdevice..o: /usr/include/gnu/stubs.h
../miscdevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../miscdevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../miscdevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../miscdevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../miscdevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../miscdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../miscdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../miscdevice..o: /usr/include/pthread.h /usr/include/sched.h
../miscdevice..o: /usr/include/signal.h /usr/include/stdint.h
../miscdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../miscdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../miscdevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../miscdevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../miscdevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../miscdevice..o: /usr/include/time.h /usr/include/unistd.h
../miscdevice..o: /usr/include/wchar.h /usr/include/xlocale.h
../miscdevice..o: /usr/lib/glib/include/glibconfig.h
../miscdevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../miscdevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../miscdevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../miscdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../miscdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../miscdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../miscdevice..o: /usr/local/include/g++-v3/bits/std_string.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../miscdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../miscdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../miscdevice..o: /usr/local/include/g++-v3/cstddef
../miscdevice..o: /usr/local/include/g++-v3/cstdio
../miscdevice..o: /usr/local/include/g++-v3/exception
../miscdevice..o: /usr/local/include/g++-v3/exception_defines.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../miscdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../miscdevice..o: /usr/local/include/g++-v3/new
../miscdevice..o: /usr/local/include/g++-v3/queue
../miscdevice..o: /usr/local/include/g++-v3/string
../miscdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../miscdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../miscdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../miscdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../miscdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../miscdevice..o: /usr/local/player-1.0-gamma/include/messages.h
../miscdevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../miscdevice..o: /usr/local/player-1.0-gamma/include/stage.h
../occupancydevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../occupancydevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../occupancydevice..o: /usr/include/bits/byteswap.h
../occupancydevice..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../occupancydevice..o: /usr/include/bits/environments.h
../occupancydevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../occupancydevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../occupancydevice..o: /usr/include/bits/local_lim.h
../occupancydevice..o: /usr/include/bits/mathcalls.h
../occupancydevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../occupancydevice..o: /usr/include/bits/posix1_lim.h
../occupancydevice..o: /usr/include/bits/posix2_lim.h
../occupancydevice..o: /usr/include/bits/posix_opt.h
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
../occupancydevice..o: /usr/include/gconv.h /usr/include/getopt.h
../occupancydevice..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../occupancydevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../occupancydevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../occupancydevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../occupancydevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../occupancydevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h
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
../occupancydevice..o: /usr/include/unistd.h /usr/include/wchar.h
../occupancydevice..o: /usr/include/xlocale.h
../occupancydevice..o: /usr/lib/glib/include/glibconfig.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../occupancydevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/fpos.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/std_string.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../occupancydevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../occupancydevice..o: /usr/local/include/g++-v3/cstddef
../occupancydevice..o: /usr/local/include/g++-v3/cstdio
../occupancydevice..o: /usr/local/include/g++-v3/exception
../occupancydevice..o: /usr/local/include/g++-v3/exception_defines.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../occupancydevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../occupancydevice..o: /usr/local/include/g++-v3/new
../occupancydevice..o: /usr/local/include/g++-v3/queue
../occupancydevice..o: /usr/local/include/g++-v3/string
../occupancydevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../occupancydevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../occupancydevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../occupancydevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../occupancydevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../occupancydevice..o: /usr/local/player-1.0-gamma/include/messages.h
../occupancydevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../occupancydevice..o: /usr/local/player-1.0-gamma/include/stage.h
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
../playerdevice..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../playerdevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../playerdevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../playerdevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../playerdevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../playerdevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/iconv.h
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
../playerdevice..o: /usr/lib/glib/include/glibconfig.h
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
../playerdevice..o: /usr/local/include/g++-v3/string
../playerdevice..o: /usr/local/include/g++-v3/typeinfo
../playerdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../playerdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../playerdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../playerdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../playerdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../playerdevice..o: /usr/local/player-1.0-gamma/include/messages.h
../playerdevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../playerdevice..o: /usr/local/player-1.0-gamma/include/stage.h
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
../player_xpsi..o: /usr/include/signal.h /usr/include/stdint.h
../player_xpsi..o: /usr/include/stdio.h /usr/include/stdlib.h
../player_xpsi..o: /usr/include/string.h /usr/include/sys/cdefs.h
../player_xpsi..o: /usr/include/sys/select.h /usr/include/sys/stat.h
../player_xpsi..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../player_xpsi..o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
../player_xpsi..o: /usr/include/time.h /usr/include/ucontext.h
../player_xpsi..o: /usr/include/unistd.h /usr/include/values.h
../player_xpsi..o: /usr/include/wchar.h /usr/include/wctype.h
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
../player_xpsi..o: /usr/local/player-1.0-gamma/include/clientproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/gpsproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/gripperproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/laserbeaconproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/laserproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/messages.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/miscproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/occupancyproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/playercclient.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/playerclient.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/playercommon.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/positionproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/ptzproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/sonarproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/speechproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/truthproxy.h
../player_xpsi..o: /usr/local/player-1.0-gamma/include/visionproxy.h
../player_xpsi..o: /usr/X11R6/include/X11/keysymdef.h
../player_xpsi..o: /usr/X11R6/include/X11/keysym.h
../player_xpsi..o: /usr/X11R6/include/X11/Xarch.h
../player_xpsi..o: /usr/X11R6/include/X11/Xfuncproto.h
../player_xpsi..o: /usr/X11R6/include/X11/X.h /usr/X11R6/include/X11/Xlib.h
../player_xpsi..o: /usr/X11R6/include/X11/Xosdefs.h
../player_xpsi..o: /usr/X11R6/include/X11/Xos.h /usr/X11R6/include/X11/Xutil.h
../positiondevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../positiondevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../positiondevice..o: /usr/include/bits/byteswap.h
../positiondevice..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../positiondevice..o: /usr/include/bits/environments.h
../positiondevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../positiondevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../positiondevice..o: /usr/include/bits/local_lim.h
../positiondevice..o: /usr/include/bits/mathcalls.h
../positiondevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../positiondevice..o: /usr/include/bits/posix1_lim.h
../positiondevice..o: /usr/include/bits/posix2_lim.h
../positiondevice..o: /usr/include/bits/posix_opt.h
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
../positiondevice..o: /usr/include/gconv.h /usr/include/getopt.h
../positiondevice..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../positiondevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../positiondevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../positiondevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../positiondevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../positiondevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h
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
../positiondevice..o: /usr/include/unistd.h /usr/include/wchar.h
../positiondevice..o: /usr/include/xlocale.h
../positiondevice..o: /usr/lib/glib/include/glibconfig.h
../positiondevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../positiondevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../positiondevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../positiondevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../positiondevice..o: /usr/local/include/g++-v3/bits/fpos.h
../positiondevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../positiondevice..o: /usr/local/include/g++-v3/bits/std_string.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../positiondevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../positiondevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../positiondevice..o: /usr/local/include/g++-v3/cstddef
../positiondevice..o: /usr/local/include/g++-v3/cstdio
../positiondevice..o: /usr/local/include/g++-v3/exception
../positiondevice..o: /usr/local/include/g++-v3/exception_defines.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../positiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../positiondevice..o: /usr/local/include/g++-v3/new
../positiondevice..o: /usr/local/include/g++-v3/queue
../positiondevice..o: /usr/local/include/g++-v3/string
../positiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../positiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../positiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../positiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../positiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../positiondevice..o: /usr/local/player-1.0-gamma/include/messages.h
../positiondevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../positiondevice..o: /usr/local/player-1.0-gamma/include/stage.h
../ptzdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../ptzdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../ptzdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../ptzdevice..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../ptzdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../ptzdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../ptzdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../ptzdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../ptzdevice..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../ptzdevice..o: /usr/include/bits/posix_opt.h
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
../ptzdevice..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../ptzdevice..o: /usr/include/gnu/stubs.h
../ptzdevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../ptzdevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../ptzdevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../ptzdevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../ptzdevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../ptzdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../ptzdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../ptzdevice..o: /usr/include/pthread.h /usr/include/sched.h
../ptzdevice..o: /usr/include/signal.h /usr/include/stdint.h
../ptzdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../ptzdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../ptzdevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../ptzdevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../ptzdevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../ptzdevice..o: /usr/include/time.h /usr/include/unistd.h
../ptzdevice..o: /usr/include/wchar.h /usr/include/xlocale.h
../ptzdevice..o: /usr/lib/glib/include/glibconfig.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../ptzdevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/std_string.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../ptzdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../ptzdevice..o: /usr/local/include/g++-v3/cstddef
../ptzdevice..o: /usr/local/include/g++-v3/cstdio
../ptzdevice..o: /usr/local/include/g++-v3/exception
../ptzdevice..o: /usr/local/include/g++-v3/exception_defines.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../ptzdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../ptzdevice..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../ptzdevice..o: /usr/local/include/g++-v3/string
../ptzdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../ptzdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../ptzdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../ptzdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../ptzdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../ptzdevice..o: /usr/local/player-1.0-gamma/include/messages.h
../ptzdevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../ptzdevice..o: /usr/local/player-1.0-gamma/include/stage.h
../puck..o: /usr/include/alloca.h /usr/include/asm/socket.h
../puck..o: /usr/include/asm/sockios.h /usr/include/assert.h
../puck..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../puck..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../puck..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../puck..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../puck..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../puck..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../puck..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../puck..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
../puck..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../puck..o: /usr/include/bits/sem.h /usr/include/bits/sigset.h
../puck..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
../puck..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
../puck..o: /usr/include/bits/time.h /usr/include/bits/types.h
../puck..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
../puck..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../puck..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../puck..o: /usr/include/features.h /usr/include/_G_config.h
../puck..o: /usr/include/gconv.h /usr/include/getopt.h
../puck..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../puck..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../puck..o: /usr/include/gtk-1.2/gdk/gdk.h /usr/include/gtk-1.2/gdk/gdkrgb.h
../puck..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkgc.h /usr/include/gtk-1.2/gtk/gtk.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../puck..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../puck..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../puck..o: /usr/include/gtk-1.2/gtk/gtklist.h
../puck..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../puck..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../puck..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../puck..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktable.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktext.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktree.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../puck..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../puck..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../puck..o: /usr/include/limits.h /usr/include/linux/limits.h
../puck..o: /usr/include/math.h /usr/include/netinet/in.h
../puck..o: /usr/include/pthread.h /usr/include/sched.h /usr/include/signal.h
../puck..o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
../puck..o: /usr/include/string.h /usr/include/sys/cdefs.h
../puck..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../puck..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../puck..o: /usr/include/sys/time.h /usr/include/sys/types.h
../puck..o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
../puck..o: /usr/include/xlocale.h /usr/lib/glib/include/glibconfig.h
../puck..o: /usr/local/include/g++-v3/bits/basic_string.h
../puck..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../puck..o: /usr/local/include/g++-v3/bits/char_traits.h
../puck..o: /usr/local/include/g++-v3/bits/concept_check.h
../puck..o: /usr/local/include/g++-v3/bits/fpos.h
../puck..o: /usr/local/include/g++-v3/bits/functexcept.h
../puck..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../puck..o: /usr/local/include/g++-v3/bits/std_cassert.h
../puck..o: /usr/local/include/g++-v3/bits/std_climits.h
../puck..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../puck..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../puck..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../puck..o: /usr/local/include/g++-v3/bits/std_cstring.h
../puck..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../puck..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../puck..o: /usr/local/include/g++-v3/bits/std_iterator.h
../puck..o: /usr/local/include/g++-v3/bits/std_memory.h
../puck..o: /usr/local/include/g++-v3/bits/std_queue.h
../puck..o: /usr/local/include/g++-v3/bits/std_string.h
../puck..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../puck..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../puck..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../puck..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../puck..o: /usr/local/include/g++-v3/bits/stl_threads.h
../puck..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../puck..o: /usr/local/include/g++-v3/bits/stl_vector.h
../puck..o: /usr/local/include/g++-v3/bits/stringfwd.h
../puck..o: /usr/local/include/g++-v3/bits/type_traits.h
../puck..o: /usr/local/include/g++-v3/cstddef /usr/local/include/g++-v3/cstdio
../puck..o: /usr/local/include/g++-v3/exception
../puck..o: /usr/local/include/g++-v3/exception_defines.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../puck..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../puck..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../puck..o: /usr/local/include/g++-v3/string
../puck..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../puck..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../puck..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../puck..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../puck..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../puck..o: /usr/local/player-1.0-gamma/include/messages.h
../puck..o: /usr/local/player-1.0-gamma/include/playercommon.h
../puck..o: /usr/local/player-1.0-gamma/include/stage.h
../raytrace..o: /usr/include/alloca.h /usr/include/assert.h
../raytrace..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../raytrace..o: /usr/include/bits/environments.h /usr/include/bits/huge_val.h
../raytrace..o: /usr/include/bits/initspin.h /usr/include/bits/local_lim.h
../raytrace..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../raytrace..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../raytrace..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
../raytrace..o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
../raytrace..o: /usr/include/bits/select.h /usr/include/bits/sigset.h
../raytrace..o: /usr/include/bits/sigthread.h /usr/include/bits/stdio_lim.h
../raytrace..o: /usr/include/bits/time.h /usr/include/bits/types.h
../raytrace..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
../raytrace..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../raytrace..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../raytrace..o: /usr/include/features.h /usr/include/_G_config.h
../raytrace..o: /usr/include/gconv.h /usr/include/getopt.h
../raytrace..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../raytrace..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../raytrace..o: /usr/include/gtk-1.2/gdk/gdk.h
../raytrace..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../raytrace..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtk.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtklist.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktable.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktext.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktree.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../raytrace..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../raytrace..o: /usr/include/limits.h /usr/include/linux/limits.h
../raytrace..o: /usr/include/math.h /usr/include/pthread.h
../raytrace..o: /usr/include/sched.h /usr/include/signal.h
../raytrace..o: /usr/include/stdint.h /usr/include/stdio.h
../raytrace..o: /usr/include/stdlib.h /usr/include/string.h
../raytrace..o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
../raytrace..o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
../raytrace..o: /usr/include/sys/types.h /usr/include/time.h
../raytrace..o: /usr/include/unistd.h /usr/include/wchar.h
../raytrace..o: /usr/include/xlocale.h /usr/lib/glib/include/glibconfig.h
../raytrace..o: /usr/local/include/g++-v3/bits/basic_string.h
../raytrace..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../raytrace..o: /usr/local/include/g++-v3/bits/char_traits.h
../raytrace..o: /usr/local/include/g++-v3/bits/concept_check.h
../raytrace..o: /usr/local/include/g++-v3/bits/fpos.h
../raytrace..o: /usr/local/include/g++-v3/bits/functexcept.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_cassert.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_climits.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_cstring.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_iterator.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_memory.h
../raytrace..o: /usr/local/include/g++-v3/bits/std_string.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_algo.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_alloc.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_construct.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_heap.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_iterator_base_funcs.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_iterator_base_types.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_iterator.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_pair.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_threads.h
../raytrace..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../raytrace..o: /usr/local/include/g++-v3/bits/stringfwd.h
../raytrace..o: /usr/local/include/g++-v3/bits/type_traits.h
../raytrace..o: /usr/local/include/g++-v3/cstddef
../raytrace..o: /usr/local/include/g++-v3/cstdio
../raytrace..o: /usr/local/include/g++-v3/exception
../raytrace..o: /usr/local/include/g++-v3/exception_defines.h
../raytrace..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../raytrace..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../raytrace..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../raytrace..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../raytrace..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../raytrace..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../raytrace..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/string
../raytrace..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../raytrace..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../raytrace..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../raytrace..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../raytrace..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../raytrace..o: /usr/local/player-1.0-gamma/include/messages.h
../raytrace..o: /usr/local/player-1.0-gamma/include/playercommon.h
../raytrace..o: /usr/local/player-1.0-gamma/include/stage.h
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
../render..o: /usr/include/wctype.h /usr/include/xlocale.h
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
../render..o: /usr/local/player-1.0-gamma/include/messages.h
../render..o: /usr/local/player-1.0-gamma/include/playercommon.h
../render..o: /usr/X11R6/include/X11/keysymdef.h
../render..o: /usr/X11R6/include/X11/keysym.h /usr/X11R6/include/X11/Xarch.h
../render..o: /usr/X11R6/include/X11/Xfuncproto.h /usr/X11R6/include/X11/X.h
../render..o: /usr/X11R6/include/X11/Xlib.h /usr/X11R6/include/X11/Xosdefs.h
../render..o: /usr/X11R6/include/X11/Xos.h /usr/X11R6/include/X11/Xutil.h
../rtk_main..o: /usr/include/alloca.h /usr/include/asm/socket.h
../rtk_main..o: /usr/include/asm/sockios.h /usr/include/assert.h
../rtk_main..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../rtk_main..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../rtk_main..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../rtk_main..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../rtk_main..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../rtk_main..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../rtk_main..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../rtk_main..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
../rtk_main..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../rtk_main..o: /usr/include/bits/sem.h /usr/include/bits/sigset.h
../rtk_main..o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
../rtk_main..o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
../rtk_main..o: /usr/include/bits/time.h /usr/include/bits/types.h
../rtk_main..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
../rtk_main..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../rtk_main..o: /usr/include/bits/xopen_lim.h /usr/include/endian.h
../rtk_main..o: /usr/include/features.h /usr/include/_G_config.h
../rtk_main..o: /usr/include/gconv.h /usr/include/getopt.h
../rtk_main..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../rtk_main..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../rtk_main..o: /usr/include/gtk-1.2/gdk/gdk.h
../rtk_main..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../rtk_main..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtk.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtklist.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktable.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktext.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktree.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../rtk_main..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../rtk_main..o: /usr/include/limits.h /usr/include/linux/limits.h
../rtk_main..o: /usr/include/math.h /usr/include/netinet/in.h
../rtk_main..o: /usr/include/pthread.h /usr/include/sched.h
../rtk_main..o: /usr/include/signal.h /usr/include/stdint.h
../rtk_main..o: /usr/include/stdio.h /usr/include/stdlib.h
../rtk_main..o: /usr/include/string.h /usr/include/sys/cdefs.h
../rtk_main..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../rtk_main..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../rtk_main..o: /usr/include/sys/time.h /usr/include/sys/types.h
../rtk_main..o: /usr/include/time.h /usr/include/unistd.h /usr/include/wchar.h
../rtk_main..o: /usr/include/xlocale.h /usr/lib/glib/include/glibconfig.h
../rtk_main..o: /usr/local/include/g++-v3/bits/basic_string.h
../rtk_main..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../rtk_main..o: /usr/local/include/g++-v3/bits/char_traits.h
../rtk_main..o: /usr/local/include/g++-v3/bits/concept_check.h
../rtk_main..o: /usr/local/include/g++-v3/bits/fpos.h
../rtk_main..o: /usr/local/include/g++-v3/bits/functexcept.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cassert.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_climits.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cstring.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_iterator.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_memory.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_queue.h
../rtk_main..o: /usr/local/include/g++-v3/bits/std_string.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_threads.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stl_vector.h
../rtk_main..o: /usr/local/include/g++-v3/bits/stringfwd.h
../rtk_main..o: /usr/local/include/g++-v3/bits/type_traits.h
../rtk_main..o: /usr/local/include/g++-v3/cstddef
../rtk_main..o: /usr/local/include/g++-v3/cstdio
../rtk_main..o: /usr/local/include/g++-v3/exception
../rtk_main..o: /usr/local/include/g++-v3/exception_defines.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../rtk_main..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../rtk_main..o: /usr/local/include/g++-v3/new /usr/local/include/g++-v3/queue
../rtk_main..o: /usr/local/include/g++-v3/string
../rtk_main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../rtk_main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../rtk_main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../rtk_main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../rtk_main..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../rtk_main..o: /usr/local/player-1.0-gamma/include/messages.h
../rtk_main..o: /usr/local/player-1.0-gamma/include/playercommon.h
../rtk_main..o: /usr/local/player-1.0-gamma/include/stage.h
../sonardevice..o: /usr/include/alloca.h /usr/include/asm/errno.h
../sonardevice..o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
../sonardevice..o: /usr/include/assert.h /usr/include/bits/byteswap.h
../sonardevice..o: /usr/include/bits/confname.h /usr/include/bits/endian.h
../sonardevice..o: /usr/include/bits/environments.h /usr/include/bits/errno.h
../sonardevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../sonardevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../sonardevice..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
../sonardevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../sonardevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../sonardevice..o: /usr/include/bits/posix2_lim.h
../sonardevice..o: /usr/include/bits/posix_opt.h
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
../sonardevice..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../sonardevice..o: /usr/include/gnu/stubs.h
../sonardevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../sonardevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../sonardevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../sonardevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../sonardevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/iconv.h
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
../sonardevice..o: /usr/include/unistd.h /usr/include/wchar.h
../sonardevice..o: /usr/include/wctype.h /usr/include/xlocale.h
../sonardevice..o: /usr/lib/glib/include/glibconfig.h
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
../sonardevice..o: /usr/local/include/g++-v3/string
../sonardevice..o: /usr/local/include/g++-v3/typeinfo
../sonardevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../sonardevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../sonardevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../sonardevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../sonardevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../sonardevice..o: /usr/local/player-1.0-gamma/include/messages.h
../sonardevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../sonardevice..o: /usr/local/player-1.0-gamma/include/stage.h
../truthdevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../truthdevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../truthdevice..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../truthdevice..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../truthdevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../truthdevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../truthdevice..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../truthdevice..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../truthdevice..o: /usr/include/bits/posix1_lim.h
../truthdevice..o: /usr/include/bits/posix2_lim.h
../truthdevice..o: /usr/include/bits/posix_opt.h
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
../truthdevice..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../truthdevice..o: /usr/include/gnu/stubs.h
../truthdevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../truthdevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../truthdevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../truthdevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../truthdevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../truthdevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../truthdevice..o: /usr/include/math.h /usr/include/netinet/in.h
../truthdevice..o: /usr/include/pthread.h /usr/include/sched.h
../truthdevice..o: /usr/include/signal.h /usr/include/stdint.h
../truthdevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../truthdevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../truthdevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../truthdevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../truthdevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../truthdevice..o: /usr/include/time.h /usr/include/unistd.h
../truthdevice..o: /usr/include/wchar.h /usr/include/xlocale.h
../truthdevice..o: /usr/lib/glib/include/glibconfig.h
../truthdevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../truthdevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../truthdevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../truthdevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../truthdevice..o: /usr/local/include/g++-v3/bits/fpos.h
../truthdevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../truthdevice..o: /usr/local/include/g++-v3/bits/std_string.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../truthdevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../truthdevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../truthdevice..o: /usr/local/include/g++-v3/cstddef
../truthdevice..o: /usr/local/include/g++-v3/cstdio
../truthdevice..o: /usr/local/include/g++-v3/exception
../truthdevice..o: /usr/local/include/g++-v3/exception_defines.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../truthdevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../truthdevice..o: /usr/local/include/g++-v3/new
../truthdevice..o: /usr/local/include/g++-v3/queue
../truthdevice..o: /usr/local/include/g++-v3/string
../truthdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../truthdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../truthdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../truthdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../truthdevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../truthdevice..o: /usr/local/player-1.0-gamma/include/messages.h
../truthdevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../truthdevice..o: /usr/local/player-1.0-gamma/include/stage.h
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
../truthlog..o: /usr/local/player-1.0-gamma/include/messages.h
../truthlog..o: /usr/local/player-1.0-gamma/include/playercommon.h
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
../truthserver..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../truthserver..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../truthserver..o: /usr/include/gtk-1.2/gdk/gdk.h
../truthserver..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../truthserver..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtk.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtklist.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktable.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktext.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktree.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../truthserver..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/iconv.h
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
../truthserver..o: /usr/include/xlocale.h /usr/lib/glib/include/glibconfig.h
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
../truthserver..o: /usr/local/include/g++-v3/string
../truthserver..o: /usr/local/include/g++-v3/typeinfo
../truthserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../truthserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../truthserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../truthserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../truthserver..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../truthserver..o: /usr/local/player-1.0-gamma/include/messages.h
../truthserver..o: /usr/local/player-1.0-gamma/include/playercommon.h
../truthserver..o: /usr/local/player-1.0-gamma/include/stage.h
../visionbeacon..o: /usr/include/alloca.h /usr/include/asm/socket.h
../visionbeacon..o: /usr/include/asm/sockios.h /usr/include/assert.h
../visionbeacon..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../visionbeacon..o: /usr/include/bits/endian.h
../visionbeacon..o: /usr/include/bits/environments.h
../visionbeacon..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../visionbeacon..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../visionbeacon..o: /usr/include/bits/local_lim.h
../visionbeacon..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../visionbeacon..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../visionbeacon..o: /usr/include/bits/posix2_lim.h
../visionbeacon..o: /usr/include/bits/posix_opt.h
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
../visionbeacon..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../visionbeacon..o: /usr/include/gnu/stubs.h
../visionbeacon..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../visionbeacon..o: /usr/include/gtk-1.2/gdk/gdk.h
../visionbeacon..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../visionbeacon..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtk.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtklist.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktable.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktext.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktree.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../visionbeacon..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../visionbeacon..o: /usr/include/limits.h /usr/include/linux/limits.h
../visionbeacon..o: /usr/include/math.h /usr/include/netinet/in.h
../visionbeacon..o: /usr/include/pthread.h /usr/include/sched.h
../visionbeacon..o: /usr/include/signal.h /usr/include/stdint.h
../visionbeacon..o: /usr/include/stdio.h /usr/include/stdlib.h
../visionbeacon..o: /usr/include/string.h /usr/include/sys/cdefs.h
../visionbeacon..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../visionbeacon..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../visionbeacon..o: /usr/include/sys/time.h /usr/include/sys/types.h
../visionbeacon..o: /usr/include/time.h /usr/include/unistd.h
../visionbeacon..o: /usr/include/wchar.h /usr/include/xlocale.h
../visionbeacon..o: /usr/lib/glib/include/glibconfig.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/basic_string.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../visionbeacon..o: /usr/local/include/g++-v3/bits/char_traits.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/concept_check.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/fpos.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/functexcept.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cassert.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_climits.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cstring.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_iterator.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_memory.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_queue.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/std_string.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_threads.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stl_vector.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/stringfwd.h
../visionbeacon..o: /usr/local/include/g++-v3/bits/type_traits.h
../visionbeacon..o: /usr/local/include/g++-v3/cstddef
../visionbeacon..o: /usr/local/include/g++-v3/cstdio
../visionbeacon..o: /usr/local/include/g++-v3/exception
../visionbeacon..o: /usr/local/include/g++-v3/exception_defines.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../visionbeacon..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../visionbeacon..o: /usr/local/include/g++-v3/new
../visionbeacon..o: /usr/local/include/g++-v3/queue
../visionbeacon..o: /usr/local/include/g++-v3/string
../visionbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../visionbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../visionbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../visionbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../visionbeacon..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../visionbeacon..o: /usr/local/player-1.0-gamma/include/messages.h
../visionbeacon..o: /usr/local/player-1.0-gamma/include/playercommon.h
../visionbeacon..o: /usr/local/player-1.0-gamma/include/stage.h
../visiondevice..o: /usr/include/alloca.h /usr/include/asm/socket.h
../visiondevice..o: /usr/include/asm/sockios.h /usr/include/assert.h
../visiondevice..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../visiondevice..o: /usr/include/bits/endian.h
../visiondevice..o: /usr/include/bits/environments.h
../visiondevice..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../visiondevice..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../visiondevice..o: /usr/include/bits/local_lim.h
../visiondevice..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../visiondevice..o: /usr/include/bits/nan.h /usr/include/bits/posix1_lim.h
../visiondevice..o: /usr/include/bits/posix2_lim.h
../visiondevice..o: /usr/include/bits/posix_opt.h
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
../visiondevice..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../visiondevice..o: /usr/include/gnu/stubs.h
../visiondevice..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../visiondevice..o: /usr/include/gtk-1.2/gdk/gdk.h
../visiondevice..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../visiondevice..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtk.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtklist.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktable.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktext.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktree.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../visiondevice..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../visiondevice..o: /usr/include/limits.h /usr/include/linux/limits.h
../visiondevice..o: /usr/include/math.h /usr/include/netinet/in.h
../visiondevice..o: /usr/include/pthread.h /usr/include/sched.h
../visiondevice..o: /usr/include/signal.h /usr/include/stdint.h
../visiondevice..o: /usr/include/stdio.h /usr/include/stdlib.h
../visiondevice..o: /usr/include/string.h /usr/include/sys/cdefs.h
../visiondevice..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../visiondevice..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../visiondevice..o: /usr/include/sys/time.h /usr/include/sys/types.h
../visiondevice..o: /usr/include/time.h /usr/include/unistd.h
../visiondevice..o: /usr/include/wchar.h /usr/include/xlocale.h
../visiondevice..o: /usr/lib/glib/include/glibconfig.h
../visiondevice..o: /usr/local/include/g++-v3/bits/basic_string.h
../visiondevice..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../visiondevice..o: /usr/local/include/g++-v3/bits/char_traits.h
../visiondevice..o: /usr/local/include/g++-v3/bits/concept_check.h
../visiondevice..o: /usr/local/include/g++-v3/bits/fpos.h
../visiondevice..o: /usr/local/include/g++-v3/bits/functexcept.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cassert.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_climits.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cstring.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_iterator.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_memory.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_queue.h
../visiondevice..o: /usr/local/include/g++-v3/bits/std_string.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_threads.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stl_vector.h
../visiondevice..o: /usr/local/include/g++-v3/bits/stringfwd.h
../visiondevice..o: /usr/local/include/g++-v3/bits/type_traits.h
../visiondevice..o: /usr/local/include/g++-v3/cstddef
../visiondevice..o: /usr/local/include/g++-v3/cstdio
../visiondevice..o: /usr/local/include/g++-v3/exception
../visiondevice..o: /usr/local/include/g++-v3/exception_defines.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../visiondevice..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../visiondevice..o: /usr/local/include/g++-v3/new
../visiondevice..o: /usr/local/include/g++-v3/queue
../visiondevice..o: /usr/local/include/g++-v3/string
../visiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../visiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../visiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../visiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../visiondevice..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../visiondevice..o: /usr/local/player-1.0-gamma/include/messages.h
../visiondevice..o: /usr/local/player-1.0-gamma/include/playercommon.h
../visiondevice..o: /usr/local/player-1.0-gamma/include/stage.h
../world_load..o: /usr/include/alloca.h /usr/include/asm/socket.h
../world_load..o: /usr/include/asm/sockios.h /usr/include/assert.h
../world_load..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../world_load..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../world_load..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../world_load..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../world_load..o: /usr/include/bits/local_lim.h /usr/include/bits/mathcalls.h
../world_load..o: /usr/include/bits/mathdef.h /usr/include/bits/nan.h
../world_load..o: /usr/include/bits/posix1_lim.h
../world_load..o: /usr/include/bits/posix2_lim.h /usr/include/bits/posix_opt.h
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
../world_load..o: /usr/include/getopt.h /usr/include/glib-1.2/glib.h
../world_load..o: /usr/include/gnu/stubs.h
../world_load..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../world_load..o: /usr/include/gtk-1.2/gdk/gdk.h
../world_load..o: /usr/include/gtk-1.2/gdk/gdkrgb.h
../world_load..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkgc.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtk.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtklist.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktable.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktext.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktree.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../world_load..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/libio.h
../world_load..o: /usr/include/limits.h /usr/include/linux/limits.h
../world_load..o: /usr/include/math.h /usr/include/netinet/in.h
../world_load..o: /usr/include/pthread.h /usr/include/sched.h
../world_load..o: /usr/include/signal.h /usr/include/stdint.h
../world_load..o: /usr/include/stdio.h /usr/include/stdlib.h
../world_load..o: /usr/include/string.h /usr/include/sys/cdefs.h
../world_load..o: /usr/include/sys/ipc.h /usr/include/sys/select.h
../world_load..o: /usr/include/sys/sem.h /usr/include/sys/sysmacros.h
../world_load..o: /usr/include/sys/time.h /usr/include/sys/types.h
../world_load..o: /usr/include/time.h /usr/include/unistd.h
../world_load..o: /usr/include/wchar.h /usr/include/xlocale.h
../world_load..o: /usr/lib/glib/include/glibconfig.h
../world_load..o: /usr/local/include/g++-v3/bits/basic_string.h
../world_load..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../world_load..o: /usr/local/include/g++-v3/bits/char_traits.h
../world_load..o: /usr/local/include/g++-v3/bits/concept_check.h
../world_load..o: /usr/local/include/g++-v3/bits/fpos.h
../world_load..o: /usr/local/include/g++-v3/bits/functexcept.h
../world_load..o: /usr/local/include/g++-v3/bits/std_algorithm.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cassert.h
../world_load..o: /usr/local/include/g++-v3/bits/std_climits.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cstddef.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cstdio.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cstdlib.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cstring.h
../world_load..o: /usr/local/include/g++-v3/bits/std_cwchar.h
../world_load..o: /usr/local/include/g++-v3/bits/std_iosfwd.h
../world_load..o: /usr/local/include/g++-v3/bits/std_iterator.h
../world_load..o: /usr/local/include/g++-v3/bits/std_memory.h
../world_load..o: /usr/local/include/g++-v3/bits/std_queue.h
../world_load..o: /usr/local/include/g++-v3/bits/std_string.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_algobase.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_algo.h
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
../world_load..o: /usr/local/include/g++-v3/bits/stl_raw_storage_iter.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_tempbuf.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_threads.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_uninitialized.h
../world_load..o: /usr/local/include/g++-v3/bits/stl_vector.h
../world_load..o: /usr/local/include/g++-v3/bits/stringfwd.h
../world_load..o: /usr/local/include/g++-v3/bits/type_traits.h
../world_load..o: /usr/local/include/g++-v3/cstddef
../world_load..o: /usr/local/include/g++-v3/cstdio
../world_load..o: /usr/local/include/g++-v3/exception
../world_load..o: /usr/local/include/g++-v3/exception_defines.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/atomicity.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++config.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/c++io.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/gthr-single.h
../world_load..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/os_defines.h
../world_load..o: /usr/local/include/g++-v3/new
../world_load..o: /usr/local/include/g++-v3/queue
../world_load..o: /usr/local/include/g++-v3/string
../world_load..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../world_load..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../world_load..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../world_load..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../world_load..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../world_load..o: /usr/local/player-1.0-gamma/include/messages.h
../world_load..o: /usr/local/player-1.0-gamma/include/playercommon.h
../world_load..o: /usr/local/player-1.0-gamma/include/stage.h
../world..o: /usr/include/alloca.h /usr/include/asm/errno.h
../world..o: /usr/include/asm/sigcontext.h /usr/include/asm/socket.h
../world..o: /usr/include/asm/sockios.h /usr/include/assert.h
../world..o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
../world..o: /usr/include/bits/endian.h /usr/include/bits/environments.h
../world..o: /usr/include/bits/errno.h /usr/include/bits/fcntl.h
../world..o: /usr/include/bits/huge_val.h /usr/include/bits/in.h
../world..o: /usr/include/bits/initspin.h /usr/include/bits/ipc.h
../world..o: /usr/include/bits/locale.h /usr/include/bits/local_lim.h
../world..o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
../world..o: /usr/include/bits/mman.h /usr/include/bits/nan.h
../world..o: /usr/include/bits/posix1_lim.h /usr/include/bits/posix2_lim.h
../world..o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
../world..o: /usr/include/bits/sched.h /usr/include/bits/select.h
../world..o: /usr/include/bits/sem.h /usr/include/bits/sigaction.h
../world..o: /usr/include/bits/sigcontext.h /usr/include/bits/siginfo.h
../world..o: /usr/include/bits/signum.h /usr/include/bits/sigset.h
../world..o: /usr/include/bits/sigstack.h /usr/include/bits/sigthread.h
../world..o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
../world..o: /usr/include/bits/stat.h /usr/include/bits/stdio_lim.h
../world..o: /usr/include/bits/time.h /usr/include/bits/types.h
../world..o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
../world..o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
../world..o: /usr/include/bits/xopen_lim.h /usr/include/ctype.h
../world..o: /usr/include/endian.h /usr/include/errno.h /usr/include/fcntl.h
../world..o: /usr/include/features.h /usr/include/_G_config.h
../world..o: /usr/include/gconv.h /usr/include/getopt.h
../world..o: /usr/include/glib-1.2/glib.h /usr/include/gnu/stubs.h
../world..o: /usr/include/gtk-1.2/gdk/gdkcursors.h
../world..o: /usr/include/gtk-1.2/gdk/gdk.h /usr/include/gtk-1.2/gdk/gdkrgb.h
../world..o: /usr/include/gtk-1.2/gdk/gdktypes.h
../world..o: /usr/include/gtk-1.2/gtk/gtkaccelgroup.h
../world..o: /usr/include/gtk-1.2/gtk/gtkaccellabel.h
../world..o: /usr/include/gtk-1.2/gtk/gtkadjustment.h
../world..o: /usr/include/gtk-1.2/gtk/gtkalignment.h
../world..o: /usr/include/gtk-1.2/gtk/gtkarg.h
../world..o: /usr/include/gtk-1.2/gtk/gtkarrow.h
../world..o: /usr/include/gtk-1.2/gtk/gtkaspectframe.h
../world..o: /usr/include/gtk-1.2/gtk/gtkbbox.h
../world..o: /usr/include/gtk-1.2/gtk/gtkbindings.h
../world..o: /usr/include/gtk-1.2/gtk/gtkbin.h
../world..o: /usr/include/gtk-1.2/gtk/gtkbox.h
../world..o: /usr/include/gtk-1.2/gtk/gtkbutton.h
../world..o: /usr/include/gtk-1.2/gtk/gtkcalendar.h
../world..o: /usr/include/gtk-1.2/gtk/gtkcheckbutton.h
../world..o: /usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
../world..o: /usr/include/gtk-1.2/gtk/gtkclist.h
../world..o: /usr/include/gtk-1.2/gtk/gtkcolorsel.h
../world..o: /usr/include/gtk-1.2/gtk/gtkcombo.h
../world..o: /usr/include/gtk-1.2/gtk/gtkcompat.h
../world..o: /usr/include/gtk-1.2/gtk/gtkcontainer.h
../world..o: /usr/include/gtk-1.2/gtk/gtkctree.h
../world..o: /usr/include/gtk-1.2/gtk/gtkcurve.h
../world..o: /usr/include/gtk-1.2/gtk/gtkdata.h
../world..o: /usr/include/gtk-1.2/gtk/gtkdebug.h
../world..o: /usr/include/gtk-1.2/gtk/gtkdialog.h
../world..o: /usr/include/gtk-1.2/gtk/gtkdnd.h
../world..o: /usr/include/gtk-1.2/gtk/gtkdrawingarea.h
../world..o: /usr/include/gtk-1.2/gtk/gtkeditable.h
../world..o: /usr/include/gtk-1.2/gtk/gtkentry.h
../world..o: /usr/include/gtk-1.2/gtk/gtkenums.h
../world..o: /usr/include/gtk-1.2/gtk/gtkeventbox.h
../world..o: /usr/include/gtk-1.2/gtk/gtkfeatures.h
../world..o: /usr/include/gtk-1.2/gtk/gtkfilesel.h
../world..o: /usr/include/gtk-1.2/gtk/gtkfixed.h
../world..o: /usr/include/gtk-1.2/gtk/gtkfontsel.h
../world..o: /usr/include/gtk-1.2/gtk/gtkframe.h
../world..o: /usr/include/gtk-1.2/gtk/gtkgamma.h
../world..o: /usr/include/gtk-1.2/gtk/gtkgc.h /usr/include/gtk-1.2/gtk/gtk.h
../world..o: /usr/include/gtk-1.2/gtk/gtkhandlebox.h
../world..o: /usr/include/gtk-1.2/gtk/gtkhbbox.h
../world..o: /usr/include/gtk-1.2/gtk/gtkhbox.h
../world..o: /usr/include/gtk-1.2/gtk/gtkhpaned.h
../world..o: /usr/include/gtk-1.2/gtk/gtkhruler.h
../world..o: /usr/include/gtk-1.2/gtk/gtkhscale.h
../world..o: /usr/include/gtk-1.2/gtk/gtkhscrollbar.h
../world..o: /usr/include/gtk-1.2/gtk/gtkhseparator.h
../world..o: /usr/include/gtk-1.2/gtk/gtkimage.h
../world..o: /usr/include/gtk-1.2/gtk/gtkinputdialog.h
../world..o: /usr/include/gtk-1.2/gtk/gtkitemfactory.h
../world..o: /usr/include/gtk-1.2/gtk/gtkitem.h
../world..o: /usr/include/gtk-1.2/gtk/gtklabel.h
../world..o: /usr/include/gtk-1.2/gtk/gtklayout.h
../world..o: /usr/include/gtk-1.2/gtk/gtklist.h
../world..o: /usr/include/gtk-1.2/gtk/gtklistitem.h
../world..o: /usr/include/gtk-1.2/gtk/gtkmain.h
../world..o: /usr/include/gtk-1.2/gtk/gtkmarshal.h
../world..o: /usr/include/gtk-1.2/gtk/gtkmenubar.h
../world..o: /usr/include/gtk-1.2/gtk/gtkmenufactory.h
../world..o: /usr/include/gtk-1.2/gtk/gtkmenu.h
../world..o: /usr/include/gtk-1.2/gtk/gtkmenuitem.h
../world..o: /usr/include/gtk-1.2/gtk/gtkmenushell.h
../world..o: /usr/include/gtk-1.2/gtk/gtkmisc.h
../world..o: /usr/include/gtk-1.2/gtk/gtknotebook.h
../world..o: /usr/include/gtk-1.2/gtk/gtkobject.h
../world..o: /usr/include/gtk-1.2/gtk/gtkoptionmenu.h
../world..o: /usr/include/gtk-1.2/gtk/gtkpacker.h
../world..o: /usr/include/gtk-1.2/gtk/gtkpaned.h
../world..o: /usr/include/gtk-1.2/gtk/gtkpixmap.h
../world..o: /usr/include/gtk-1.2/gtk/gtkplug.h
../world..o: /usr/include/gtk-1.2/gtk/gtkpreview.h
../world..o: /usr/include/gtk-1.2/gtk/gtkprogressbar.h
../world..o: /usr/include/gtk-1.2/gtk/gtkprogress.h
../world..o: /usr/include/gtk-1.2/gtk/gtkradiobutton.h
../world..o: /usr/include/gtk-1.2/gtk/gtkradiomenuitem.h
../world..o: /usr/include/gtk-1.2/gtk/gtkrange.h
../world..o: /usr/include/gtk-1.2/gtk/gtkrc.h
../world..o: /usr/include/gtk-1.2/gtk/gtkruler.h
../world..o: /usr/include/gtk-1.2/gtk/gtkscale.h
../world..o: /usr/include/gtk-1.2/gtk/gtkscrollbar.h
../world..o: /usr/include/gtk-1.2/gtk/gtkscrolledwindow.h
../world..o: /usr/include/gtk-1.2/gtk/gtkselection.h
../world..o: /usr/include/gtk-1.2/gtk/gtkseparator.h
../world..o: /usr/include/gtk-1.2/gtk/gtksignal.h
../world..o: /usr/include/gtk-1.2/gtk/gtksocket.h
../world..o: /usr/include/gtk-1.2/gtk/gtkspinbutton.h
../world..o: /usr/include/gtk-1.2/gtk/gtkstatusbar.h
../world..o: /usr/include/gtk-1.2/gtk/gtkstyle.h
../world..o: /usr/include/gtk-1.2/gtk/gtktable.h
../world..o: /usr/include/gtk-1.2/gtk/gtktearoffmenuitem.h
../world..o: /usr/include/gtk-1.2/gtk/gtktext.h
../world..o: /usr/include/gtk-1.2/gtk/gtkthemes.h
../world..o: /usr/include/gtk-1.2/gtk/gtktipsquery.h
../world..o: /usr/include/gtk-1.2/gtk/gtktogglebutton.h
../world..o: /usr/include/gtk-1.2/gtk/gtktoolbar.h
../world..o: /usr/include/gtk-1.2/gtk/gtktooltips.h
../world..o: /usr/include/gtk-1.2/gtk/gtktree.h
../world..o: /usr/include/gtk-1.2/gtk/gtktreeitem.h
../world..o: /usr/include/gtk-1.2/gtk/gtktypebuiltins.h
../world..o: /usr/include/gtk-1.2/gtk/gtktypeutils.h
../world..o: /usr/include/gtk-1.2/gtk/gtkvbbox.h
../world..o: /usr/include/gtk-1.2/gtk/gtkvbox.h
../world..o: /usr/include/gtk-1.2/gtk/gtkviewport.h
../world..o: /usr/include/gtk-1.2/gtk/gtkvpaned.h
../world..o: /usr/include/gtk-1.2/gtk/gtkvruler.h
../world..o: /usr/include/gtk-1.2/gtk/gtkvscale.h
../world..o: /usr/include/gtk-1.2/gtk/gtkvscrollbar.h
../world..o: /usr/include/gtk-1.2/gtk/gtkvseparator.h
../world..o: /usr/include/gtk-1.2/gtk/gtkwidget.h
../world..o: /usr/include/gtk-1.2/gtk/gtkwindow.h /usr/include/iconv.h
../world..o: /usr/include/langinfo.h /usr/include/libio.h
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
../world..o: /usr/lib/glib/include/glibconfig.h
../world..o: /usr/local/include/g++-v3/backward/backward_warning.h
../world..o: /usr/local/include/g++-v3/backward/fstream.h
../world..o: /usr/local/include/g++-v3/backward/iostream.h
../world..o: /usr/local/include/g++-v3/bits/basic_file.h
../world..o: /usr/local/include/g++-v3/bits/basic_ios.h
../world..o: /usr/local/include/g++-v3/bits/basic_ios.tcc
../world..o: /usr/local/include/g++-v3/bits/basic_string.h
../world..o: /usr/local/include/g++-v3/bits/basic_string.tcc
../world..o: /usr/local/include/g++-v3/bits/char_traits.h
../world..o: /usr/local/include/g++-v3/bits/codecvt.h
../world..o: /usr/local/include/g++-v3/bits/concept_check.h
../world..o: /usr/local/include/g++-v3/bits/fpos.h
../world..o: /usr/local/include/g++-v3/bits/fstream.tcc
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
../world..o: /usr/local/include/g++-v3/bits/std_fstream.h
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
../world..o: /usr/local/include/g++-v3/i686-pc-linux-gnu/bits/basic_file_model.h
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
../world..o: /usr/local/include/g++-v3/string
../world..o: /usr/local/include/g++-v3/typeinfo
../world..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/float.h
../world..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/limits.h
../world..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stdarg.h
../world..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/stddef.h
../world..o: /usr/local/lib/gcc-lib/i686-pc-linux-gnu/3.0/include/syslimits.h
../world..o: /usr/local/player-1.0-gamma/include/messages.h
../world..o: /usr/local/player-1.0-gamma/include/playercommon.h
../world..o: /usr/local/player-1.0-gamma/include/stage.h
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
../xs..o: /usr/include/xlocale.h
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
../xs..o: /usr/local/player-1.0-gamma/include/messages.h
../xs..o: /usr/local/player-1.0-gamma/include/playercommon.h
../xs..o: /usr/X11R6/include/X11/keysymdef.h /usr/X11R6/include/X11/keysym.h
../xs..o: /usr/X11R6/include/X11/Xarch.h /usr/X11R6/include/X11/Xfuncproto.h
../xs..o: /usr/X11R6/include/X11/X.h /usr/X11R6/include/X11/Xlib.h
../xs..o: /usr/X11R6/include/X11/Xosdefs.h /usr/X11R6/include/X11/Xos.h
../xs..o: /usr/X11R6/include/X11/Xutil.h
