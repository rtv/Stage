
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.85 2004-12-30 04:39:24 rtv Exp $

# install these headers
include_HEADERS = stage.h 

AM_CFLAGS = -Wall -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@
AM_CPPFLAGS = -Wall -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@

lib_LTLIBRARIES = libstage.la

# The stage library contains everything you need to build a robot
# simulation, including graphics and a world description file
# parser. It also contains hooks for Player, so you can load it as a
# Player plugin.
#
libstage_la_SOURCES = \
	bitmap.c \
	gui.c \
	gui.h \
	gui_menus.c \
	matrix.c \
	model.c \
	model_blobfinder.c \
	model_fiducial.c \
	model_laser.c \
	model_pose.c \
	model_position.c \
	model_props.c \
	model_ranger.c \
	raytrace.c \
	stage.c \
	stage.h \
	stage_internal.h \
	stagecpp.cc \
	stg_driver.cc \
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


# todo
#	model_energy.c
#	token.c 
