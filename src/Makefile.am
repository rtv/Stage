
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.66 2004-09-24 20:58:30 rtv Exp $

#SUBDIRS = models

#stage_DEPENDENCIES = libstage.a $(top_srcdir)/replace/libreplace.a 

# install these headers
include_HEADERS = stage.h 

AM_CFLAGS = -Wall -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@
AM_CPPFLAGS = -Wall -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@

lib_LTLIBRARIES = libstage.la libstageplugin.la 

# the stage library contains everything you need to build a robot
# simulation, including a graphics library and and worldfile parser
#
libstage_la_SOURCES = \
	bitmap.c \
	gui.c \
	gui.h \
	gui_menus.c \
	library.c \
	matrix.c \
	matrix.h \
	model.c \
	model.h \
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
	raytrace.h \
	stage.c \
	stage.h \
	stagecpp.cc \
	token.c \
	world.c \
	world.h \
	worldfile.cc \
	worldfile.hh \
        colors.h \
        rtk.c \
        rtk.h \
        rtk_canvas.c \
        rtk_fig.c \
        rtk_menu.c \
        rtk_region.c \
        rtkprivate.h 

libstage_la_LDFLAGS = -export-dynamic -version-info 0:0:0 

# the stageplugin library includes the main stage library, plus Player driver code.
#
libstageplugin_la_SOURCES = \
        stg_driver.cc \
        stg_driver.h \
	stg_time.cc \
	stg_time.h \
	stg_simulation.cc \
	stg_laser.cc 

libstageplugin_la_DEPENDENCIES = libstage.la
libstageplugin_la_LDFLAGS = -module -export-dynamic -version-info 0:0:0 
libstageplugin_la_LIBADD = @GTK_LIBS@ -lstage


bin_PROGRAMS = stest

# test program	
#	
stest_SOURCES = stest.c
stest_LDFLAGS =
stest_LDADD =  @GTK_LIBS@ -lstage -ljpeg



