
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.21 2002-10-31 01:32:00 gerkey Exp $


SUBDIRS = models @GUI_DIR@

# we need extra flags as long as RTK code is embedded in the main simulator source
if WITH_RTK_GUI
if HAVE_GTK
RTKCPPFLAGS = -I$(top_srcdir)/rtk2 `gtk-config --cflags`
endif
endif

stage_DEPENDENCIES = $(NOGUIOBJ)

AM_CPPFLAGS = 	-Wall -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage $(RTKCPPFLAGS)

bin_PROGRAMS = stage

stage_SOURCES = \
		client.cc			\
		colors.cc			\
		colors.hh			\
		entity.cc			\
		entity.hh			\
		gui.hh				\
		library.cc			\
		library.hh			\
		main.cc				\
		matrix.cc			\
		matrix.hh			\
		playerdevice.cc			\
		playerdevice.hh			\
		raytrace.cc			\
		raytrace.hh			\
		server.cc			\
		server.hh			\
		stage_types.hh			\
		stageio.cc			\
		world.cc			\
		world.hh			\
		worldfile.cc			\
		worldfile.hh			\
		nogui.cc

stage_LDFLAGS = 

stage_LDADD = 	-lm -Lmodels -lstgmodels -L@PLAYER_DIR@/lib -L@PLAYER_DIR@/server -lplayerqueue @GUI_LINK@ @LIBOBJS@
