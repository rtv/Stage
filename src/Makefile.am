
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.29.4.5 2003-02-03 07:10:36 rtv Exp $


#SUBDIRS = models rtkstage gnome2
SUBDIRS = stageio test

# we need extra flags as long as RTK code is embedded in the main simulator source
if WITH_RTK
RTKCPPFLAGS = -I@RTK_DIR@/include `gtk-config --cflags`
endif


#stage_DEPENDENCIES = $(NOGUIOBJ) models/libstgmodels.a $(top_srcdir)/replace/libreplace.a  @GUI_DIR@/@GUI_LIB@ 

stage_DEPENDENCIES = stageio/libstageio.a $(top_srcdir)/replace/libreplace.a  $(NOGUIOBJ) @GUI_DIR@/@GUI_LIB@ 

#AM_CPPFLAGS = 	-Wall -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage -I@PLAYER_DIR@/client_libs/libplayerpacket -I$(top_builddir)/replace $(RTKCPPFLAGS)

AM_CPPFLAGS = 	-Wall -Istageio -I$(top_builddir)/replace $(RTKCPPFLAGS)

bin_PROGRAMS = stage

stage_SOURCES = \
	entity.cc \
	entity.hh \
	main.cc \
	matrix.cc \
	matrix.hh \
	nogui.cc \
	raytrace.cc \
	root.cc \
	root.hh \
	colors.cc \
	colors.hh \
	library.cc \
	library.hh \
	raytrace.hh

stage_LDFLAGS = 
stage_LDADD = @GUI_LINK@ -lm -Lstageio -lstageio -L$(top_builddir)/replace -lreplace

#stage_LDADD = 	-lm -Lmodels -lstgmodels -L@PLAYER_DIR@/lib -L@PLAYER_DIR@/server -L@PLAYER_DIR@/client_libs/libplayerpacket -lplayerqueue -lplayerpacket @GUI_LINK@ -L$(top_builddir)/replace -lreplace

