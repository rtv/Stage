
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.76 2004-11-08 20:34:22 rtv Exp $

# we don't need these right now, but I'll keep this commented out just
# in case.  
# stage_DEPENDENCIES = $(top_srcdir)/replace/libreplace.a

# install these headers
include_HEADERS = stage.h 

AM_CFLAGS = -Wall -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@
AM_CPPFLAGS = -Wall -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@

lib_LTLIBRARIES = libstage.la 

# the stage library contains everything you need to build a robot
# simulation, including a graphics library and and worldfile parser
#
libstage_la_SOURCES = \
	bitmap.c \
	gui.c \
	gui.h \
	gui_menus.c \
	matrix.c \
	model.c \
	model_blobfinder.c \
	model_energy.c \
	model_fiducial.c \
	model_laser.c \
	model_pose.c \
	model_position.c \
	model_props.c \
	model_ranger.c \
	model_test.c \
	raytrace.c \
	stage.c \
	stage.h \
	stagecpp.cc \
	token.c \
	world.c \
	worldfile.cc \
	worldfile.hh \
        rtk.c \
        rtk.h \
        rtk_canvas.c \
        rtk_fig.c \
        rtk_menu.c \
        rtk_region.c \
        rtkprivate.h \
        stg_driver.cc \
        stg_driver.h \
	stg_time.cc \
	stg_time.h \
	stg_simulation.cc \
	stg_laser.cc \
	stg_fiducial.cc \
	stg_position.cc \
	stg_sonar.cc \
	stg_blobfinder.cc

libstage_la_LDFLAGS = -export-dynamic -version-info 0:0:0
libstage_la_LIBADD = @GTK_LIBS@ 

