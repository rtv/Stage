
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.110 2005-08-09 02:30:28 rtv Exp $

# install the header file for the Stage library
include_HEADERS = stage.h 

#AM_CFLAGS = -Wall -I. -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GLIB_CFLAGS@ @GTK_CFLAGS@
#AM_CPPFLAGS = -Wall -I. -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GLIB_LIBS@ @GTK_CFLAGS@
AM_CFLAGS = -std=c99 -Wall -I. -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@
AM_CPPFLAGS = -Wall -I. -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@

# build the stage library and the stage plugin
lib_LTLIBRARIES = libstage.la libstageplugin.la

# build stest, a stand-alone test binary using the stage library, 
# and ptest, a player client for testing the player plugin
bin_PROGRAMS = stest ptest


# The stage library contains everything you need to build a robot
# simulation, including graphics and a world description file
# parser. It does NOT contain a Player interface, so it can not be
# used as a player plugin. See libstageplugin (below) for good Player
# satisfaction.
#
libstage_la_SOURCES = \
	../config.h \
	gui.c \
	gui.h \
	gui_menus.c \
	matrix.c \
	model.c \
	model_blobfinder.c \
	model_fiducial.c \
	model_gripper.c \
	model_laser.c \
	model_position.c \
	model_ranger.c \
	raytrace.c \
	stage.c \
	stage.h \
	stage_internal.h \
	stagecpp.cc \
	typetable.c \
	world.c \
	worldfile.cc \
	worldfile.hh \
        rtk.c \
        rtk.h \
        rtk_canvas.c \
        rtk_fig.c \
        rtk_region.c \
        rtkprivate.h

# TODO
#	model_energy.c

libstage_la_LDFLAGS = -version-info 2:0:0 
libstage_la_LIBADD = @GTK_LIBS@

# build a Player plugin from libstage plus Player driver hooks.
libstageplugin_la_SOURCES = \
	p_blobfinder.cc \
	p_driver.cc \
	p_driver.h \
	p_energy.cc \
	p_fiducial.cc \
	p_gripper.cc \
	p_laser.cc \
	p_map.cc \
	p_position.cc \
	p_simulation.cc \
	p_sonar.cc \
	stg_time.cc \
	stg_time.h \
	zoo_driver.cc \
	zoo_driver.h 

libstageplugin_la_LDFLAGS = -export-dynamic -version-info 1:0:0 
libstageplugin_la_LIBADD = -L.libs -lstage
#libstageplugin_la_LIBADD = a-lstage

# build a stand-alone test program using libstage
stest_DEPENDENCIES = libstage.la
stest_SOURCES = stest.cc
stest_LDADD = libstage.la

# build a stand-alone test program using libstage
#ptest_DEPENDENCIES = 
ptest_SOURCES = ptest.c
ptest_CFLAGS = @PLAYERC_CFLAGS@ -std=c99
ptest_LDADD = @PLAYERC_LIBS@
