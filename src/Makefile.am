
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.26 2002-11-19 04:27:18 rtv Exp $


SUBDIRS = models rtkstage gnome2

# we need extra flags as long as RTK code is embedded in the main simulator source
if WITH_RTK
RTKCPPFLAGS = -I@RTK_DIR@/include `gtk-config --cflags`
endif

stage_DEPENDENCIES = $(NOGUIOBJ) models/libstgmodels.a  @GUI_DIR@/@GUI_LIB@

AM_CPPFLAGS = 	-Wall -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage -I@PLAYER_DIR@/client_libs/libplayerpacket $(RTKCPPFLAGS)

bin_PROGRAMS = stage


stage_SOURCES = \
         client.cc \
         colors.cc \
         colors.hh \
         entity.cc \
         entity.hh \
         gui.hh \
         library.cc \
         library.hh \
         main.cc \
         matrix.cc \
         matrix.hh \
         nogui.cc \
         playerdevice.cc \
         playerdevice.hh \
         raytrace.cc \
         raytrace.hh \
         replace.h \
         server.cc \
         server.hh \
         stageio.cc \
         stage_types.hh \
         world.cc \
         worldfile.cc \
         worldfile.hh \
         world.hh

stage_LDFLAGS =
stage_LDADD = 	-lm -Lmodels -lstgmodels -L@PLAYER_DIR@/lib -L@PLAYER_DIR@/server -L@PLAYER_DIR@/client_libs/libplayerpacket -lplayerqueue -lplayerpacket @GUI_LINK@ @LIBOBJS@
