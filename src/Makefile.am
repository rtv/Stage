
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.140.2.4 2007-03-03 02:20:52 rtv Exp $

# also build Zoo referees
SUBDIRS = referees

# install the header file for the Stage library
include_HEADERS = stage.h 

# system-wide compile flags - target-specific flags are added to each target below
AM_CFLAGS =   -Wall -I. -I$(top_srcdir)/replace -std=c99 @GUI_CFLAGS@ 
AM_CPPFLAGS = -Wall -I. -I$(top_srcdir)/replace  @GUI_CFLAGS@

# build the stage library
lib_LTLIBRARIES = libstage.la 

# TODO put this back in before release
# build stest, a stand-alone test binary using the stage library, 
#bin_PROGRAMS = stest 

# The stage library contains everything you need to build a robot
# simulation, including graphics and a world description file
# parser. It does NOT contain a Player interface, so it can not be
# used as a player plugin. See libstageplugin (below) for good Player
# satisfaction.
#
libstage_la_SOURCES =  \
	../config.h \
	gui.h \
	gui_gl.c \
	gui_gtk.c \
	gui_gtk_prefs.c \
	matrix.c \
	model.c \
	model_blobfinder.c \
	model_callbacks.c \
	model_bumper.c \
	model_fiducial.c \
	model_laser.c \
	model_load.c \
	model_position.c \
	model_ptz.c \
	model_ranger.c \
	model_speech.c \
	model_wifi.cc \
	raytrace.c \
	stage.c \
	stage.h \
	stage_internal.h \
	stagecpp.cc \
	typetable.c \
	world.c \
	worldfile.cc \
	worldfile.hh 


# TODO
# model_power.c # needs lots of work
#	model_gripper.c 

libstage_la_LDFLAGS = -version-info 2:1:0 -rpath $(libdir)  
libstage_la_LIBADD = @GUI_LIBS@


# if Player is present, build the plugin and the test program
if HAVE_PLAYER
  lib_LTLIBRARIES += libstageplugin.la
  # bin_PROGRAMS += ptest                  #currently broken
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
	p_sonar.cc \
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
# p_gripper.cc # slightly broken by PADI-2.1 changes
# p_power.cc 
# p_graphics2d.cc

# TODO fix these and move them to a separate tree?
#zoo.c 
#zoo_driver.cc 
#zoo_species.cc 
#zoo_controller.cc 
#zoo_referee.cc 

libstageplugin_la_CPPFLAGS = $(AM_CPPFLAGS) @PLAYER_CFLAGS@
libstageplugin_la_LDFLAGS = -version-info 1:0:0 -rpath $(libdir)
libstageplugin_la_LIBADD = libstage.la @PLAYER_LIBS@ @GUI_LIBS@

# build a stand-alone test program using libstage
stest_DEPENDENCIES = libstage.la
stest_SOURCES = stest.c
stest_LDADD = libstage.la 
#stest_LDFLAGS = 

# TODO - fix this
# build a player client that tests the plugin
#ptest_DEPENDENCIES = 
#ptest_SOURCES = ptest.c
#ptest_CFLAGS = $(AM_CFLAGS) @PLAYER_CFLAGS@ 
#ptest_LDADD = @PLAYERC_LIBS@
