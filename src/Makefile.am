
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.29.4.15 2003-02-13 02:26:07 rtv Exp $


#SUBDIRS = models rtkstage gnome2
SUBDIRS = sio rtkgui models

# we need extra flags as long as RTK code is embedded in the main simulator source
if WITH_RTK
RTKCPPFLAGS = -I@RTK_DIR@/include `gtk-config --cflags`
endif


#stage_DEPENDENCIES = $(NOGUIOBJ) models/libstgmodels.a $(top_srcdir)/replace/libreplace.a  @GUI_DIR@/@GUI_LIB@ 

stage_DEPENDENCIES = models/libmodels.a sio/libsio.a $(top_srcdir)/replace/libreplace.a  @GUI_DIR@/@GUI_LIB@ 

#AM_CPPFLAGS = 	-Wall -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage -I@PLAYER_DIR@/client_libs/libplayerpacket -I$(top_builddir)/replace $(RTKCPPFLAGS)

AM_CPPFLAGS = -Wall -Isio -I$(top_builddir)/replace $(RTKCPPFLAGS)

bin_PROGRAMS = stage stest

stage_SOURCES = \
	colors.cc \
	colors.hh \
	entity.cc \
	entity.hh \
	gui.h \
	library.cc \
	library.hh \
	main.cc \
	matrix.cc \
	matrix.hh \
	raytrace.cc \
	raytrace.hh \
	raytrace.hh \
	stage.h

stage_LDFLAGS = 
stage_LDADD = @GUI_LINK@ -lm -Lmodels -lmodels -Lsio -lsio -L$(top_builddir)/replace -lreplace 

# build Stage's test program - a simple client
#	
stest_DEPENDENCIES = sio/libsio.a $(top_srcdir)/replace/libreplace.a
stest_SOURCES = stest.c 
stest_LDADD = -lpnm -lm -Lsio -lsio -L$(top_builddir)/replace -lreplace 

#stage_LDADD = 	-lm -Lmodels -lmodels -L@PLAYER_DIR@/lib -L@PLAYER_DIR@/server -L@PLAYER_DIR@/client_libs/libplayerpacket -lplayerqueue -lplayerpacket @GUI_LINK@ -L$(top_builddir)/replace -lreplace

