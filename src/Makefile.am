
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.92 2005-02-26 09:07:22 rtv Exp $

# install these headers
include_HEADERS = stage.h 

AM_CFLAGS = -Wall -I. -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@
AM_CPPFLAGS = -Wall -I. -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@

lib_LTLIBRARIES = libstage.la

# The stage library contains everything you need to build a robot
# simulation, including graphics and a world description file
# parser. It also contains hooks for Player, so you can load it as a
# Player plugin.
#
libstage_la_SOURCES = \
	gui.c \
	gui.h \
	gui_menus.c \
	matrix.c \
	model.c \
	model_blobfinder.c \
	model_energy.c \
	model_fiducial.c \
	model_laser.c \
	model_position.c \
	model_ranger.c \
	player_driver.cc \
	player_interfaces.cc \
	raytrace.c \
	stage.c \
	stage.h \
	stage_internal.h \
	stagecpp.cc \
	stg_time.cc \
	stg_time.h \
	world.c \
	worldfile.cc \
	worldfile.hh \
        rtk.c \
        rtk.h \
        rtk_canvas.c \
        rtk_fig.c \
        rtk_region.c \
        rtkprivate.h

libstage_la_LDFLAGS = -export-dynamic -version-info 0:0:0 
libstage_la_LIBADD = @GTK_LIBS@
