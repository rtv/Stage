
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.142 2006-07-28 22:04:01 pooya Exp $

# also build Zoo referees
SUBDIRS = referees

# install the header file for the Stage library
include_HEADERS = stage.h 

# system-wide compile flags - target-specific flags are added to each target below
AM_CFLAGS =   -Wall -I. -I$(top_srcdir)/replace @GTK_CFLAGS@  -std=c99 
AM_CPPFLAGS = -Wall -I. -I$(top_srcdir)/replace @GTK_CFLAGS@

# build the stage library
lib_LTLIBRARIES = libstage.la 

# if Player is present, build the plugin and the test program
if HAVE_PLAYER
  lib_LTLIBRARIES += libstageplugin.la
endif

# The stage library contains everything you need to build a robot
# simulation, including graphics and a world description file
# parser. It does NOT contain a Player interface, so it can not be
# used as a player plugin. See libstageplugin (below) for good Player
# satisfaction.
#
libstage_la_SOURCES = \
	../config.h \
	gui.h \
	gui_prefs.c \
	matrix.c \
	model.c \
	model_blobfinder.c \
	model_bumper.c \
	model_laser.c \
	model_load.c \
	model_position.c \
	model_fiducial.c \
	model_ranger.c \
	model_gripper.c \
	model_ptz.c \
	model_wifi.cc \
	model_speech.c \
	model_audio.c \
	dijkstra.c \
	dijkstra.h \
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
	rtkprivate.h \
	gui_menus.c gui.c 

# TODO
# model_power.c

libstage_la_LDFLAGS = -version-info 2:0:0 -rpath $(libdir)
libstage_la_LIBADD = @GTK_LIBS@

if GNOME
AM_CFLAGS += @GNOME_CFLAGS@
AM_CPPFLAGS += @GNOME_CFLAGS@
libstage_la_SOURCES += gnome.c gnome-canvas-acetate.c gnome-canvas-acetate.h
libstage_la_LIBADD += @GNOME_LIBS@
else
libstage_la_SOURCES += 
endif

# build a Player plugin from libstage plus Player driver hooks.
libstageplugin_la_SOURCES = \
	p_bumper.cc \
	p_fiducial.cc \
	p_driver.cc \
	p_driver.h \
	p_laser.cc \
	p_ptz.cc \
	p_wifi.cc \
	p_speech.cc \
	p_audio.cc \
	p_sonar.cc \
	p_graphics2d.cc \
	p_gripper.cc \
	p_localize.cc \
	p_position.cc \
	p_map.cc \
	p_simulation.cc \
	p_blobfinder.cc \
	stg_time.cc \
	stg_time.h \
	zoo_driver.h \
	zoo.h \
	zoo_controller.h \
	zoo_referee.h \
	zoo_species.h

# TODO
# p_power.cc 


# TODO fix these and move them to a separate tree?
#zoo.c 
#zoo_driver.cc 
#zoo_species.cc 
#zoo_controller.cc 
#zoo_referee.cc 

libstageplugin_la_CPPFLAGS = $(AM_CPPFLAGS) @PLAYER_CFLAGS@  
libstageplugin_la_LDFLAGS = -version-info 1:0:0 -rpath $(libdir)
libstageplugin_la_LIBADD = libstage.la @PLAYER_LIBS@

