
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.63 2004-09-18 00:10:12 rtv Exp $

#SUBDIRS = models

#stage_DEPENDENCIES = libstage.a $(top_srcdir)/replace/libreplace.a 

AM_CFLAGS = -Wall -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@
AM_CPPFLAGS = -Wall -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@


bin_PROGRAMS = stest

# install these headers
include_HEADERS = stage.h 
#  world.h model.h matrix.h server.h connection.h gui.h

#
# stage program
#
#stage_SOURCES = \
#	connection.c \
#	connection.h \
#	main.c \
#	server.c \
#	server.h \
#	subscription.c \
#	subscription.h 

lib_LTLIBRARIES = libstage.la libstageplugin.la 

# the stage library contains almost everything you need to build a
# robot simulation, including a graphics library and and worldfile parser
#
libstage_la_SOURCES = \
	bitmap.c \
        colors.h \
	gui.c \
	gui.h \
	gui_menus.c \
	library.c \
	matrix.c \
	matrix.h \
	model.c \
	model.h \
	model_blobfinder.c \
	model_laser.c \
	model_pose.c \
	model_position.c \
	model_props.c \
	model_render.c \
	model_test.c \
	raytrace.c \
	raytrace.h \
	stagecpp.cc \
	stage.c \
	stage.h \
	token.c \
	world.c \
	world.h \
        rtk.c \
        rtk_canvas.c \
        rtk_canvas_movie.c \
        rtk_fig.c \
        rtk_region.c \
        rtk.h \
        rtk_menu.c \
        rtkprivate.h \
        rtk_table.c \
	worldfile.cc \
	worldfile.hh \
	model_energy.c \
	model_fiducial.c \
	model_ranger.c 



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

#
# test program	
#	
stest_SOURCES = stest.c
stest_LDFLAGS =
stest_LDADD =  @GTK_LIBS@ -lstage



