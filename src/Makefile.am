
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.15 2002-10-27 21:55:37 rtv Exp $


SUBDIRS = models @GUI_DIR@

# we need extra flags as long as RTK code is embedded in the main simulator source
if WITH_GUI
RTKCPPFLAGS = -I$(top_srcdir)/src/rtk2 `gtk-config --cflags`
endif

AM_CPPFLAGS = 	-Wall \
		-I@PLAYER_DIR@/include \
		-I@PLAYER_DIR@/server \
		-I@PLAYER_DIR@/server/drivers/stage \
		$(RTKCPPFLAGS)

bin_PROGRAMS = stage

stage_DEPENDENCIES = models/libstgmodels.a @GUI_DIR@/@GUI_LIB@

stage_SOURCES = \
		client.cc			\
		colors.cc			\
		colors.hh			\
		entity.cc			\
		entity.hh			\
		library.cc			\
		library.hh			\
		main.cc				\
		matrix.cc			\
		matrix.hh			\
		playerdevice.cc			\
		playerdevice.hh			\
		raytrace.cc			\
		raytrace.hh			\
                rtkgui.cc                       \
		server.cc			\
		server.hh			\
		stage_types.hh			\
		stageio.cc			\
		world.cc			\
		world.hh			\
		worldfile.cc			\
		worldfile.hh                    

stage_LDFLAGS = 

stage_LDADD = -lm -Lmodels -lstgmodels \
		-L@PLAYER_DIR@/lib -L@PLAYER_DIR@/server -lplayerqueue \
		@LIBOBJS@ -L@GUI_DIR@ -l@GUI_LIB_NAME@ @GUI_PKG_LINK@

