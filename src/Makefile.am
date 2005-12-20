
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.127 2005-12-20 21:30:22 rtv Exp $

# also build Zoo referees
SUBDIRS = referees

# install the header file for the Stage library
include_HEADERS = stage.h 

# system-wide compile flags - target-specific flags are added to each target below
AM_CFLAGS =   -Wall -I. -I$(top_builddir)/replace @GTK_CFLAGS@ @GLIB_CFLAGS@ @GNOME_CFLAGS@ -std=c99 
AM_CPPFLAGS = -Wall -I. -I$(top_builddir)/replace @GTK_CFLAGS@ @GLIB_CFLAGS@ @GNOME_CFLAGS@

# build the stage library and the stage plugin
lib_LTLIBRARIES = libstage.la libstageplugin.la

# build stest, a stand-alone test binary using the stage library, 
# and ptest, a player client for testing the player plugin
bin_PROGRAMS = stest 
#ptest

# The stage library contains everything you need to build a robot
# simulation, including graphics and a world description file
# parser. It does NOT contain a Player interface, so it can not be
# used as a player plugin. See libstageplugin (below) for good Player
# satisfaction.
#
libstage_la_SOURCES = \
	../config.h \
	gnome.c \
	gui.c \
	gui.h \
	gui_menus.c \
	matrix.c \
	model.c \
	model_blobfinder.c \
	model_laser.c \
	model_load.c \
	model_position.c \
	model_fiducial.c \
	model_ranger.c \
	model_gripper.c \
	model_energy.c \
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


libstage_la_LDFLAGS = -version-info 2:0:0 -rpath $(libdir)
libstage_la_LIBADD = @GTK_LIBS@ @GLIB_LIBS@ @GNOME_LIBS@

# build a Player plugin from libstage plus Player driver hooks.
libstageplugin_la_SOURCES = \
	p_fiducial.cc \
	p_driver.cc \
	p_driver.h \
	p_laser.cc \
	p_sonar.cc \
	p_gripper.cc \
	p_localize.cc \
	p_position.cc \
	p_power.cc \
	p_map.cc \
	p_simulation.cc \
	p_blobfinder.cc \
	stg_time.cc \
	stg_time.h 

# broken?
	zoo.h \
	zoo_driver.h \
        zoo_controller.h \
        zoo_referee.h \
        zoo_species.h


#zoo.c 
#zoo_driver.cc 
#zoo_species.cc 
#zoo_controller.cc 
#zoo_referee.cc 

libstageplugin_la_CPPFLAGS = $(AM_CPPFLAGS) @PLAYER_CFLAGS@  
libstageplugin_la_LDFLAGS = -version-info 1:0:0 -rpath $(libdir)
libstageplugin_la_LIBADD = libstage.la @PLAYER_LIBS@

# build a stand-alone test program using libstage
stest_DEPENDENCIES = libstage.la
stest_SOURCES = stest.c
stest_LDADD = libstage.la

# build a stand-alone test program using libstage
#ptest_DEPENDENCIES = 
#ptest_SOURCES = ptest.c
#ptest_CFLAGS = $(AM_CFLAGS) @PLAYER_CFLAGS@ 
#ptest_LDADD = @PLAYERC_LIBS@
