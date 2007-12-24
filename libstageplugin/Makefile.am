# Automake script to build Libstageplugin's Makefile
#
# RTV 20071126
# $Id: Makefile.am,v 1.1.2.3 2007-12-24 11:20:37 rtv Exp $

SUBDIRS = 

# system-wide compile flags - target-specific flags are added to each target below
AM_CPPFLAGS = -Wall -I. -I../libstage -I$(top_srcdir)/replace  @PIXBUF_CFLAGS@ @FLTK_CFLAGS@

# if Player is present, build the plugin and the test program
if HAVE_PLAYER
  lib_LTLIBRARIES = libstageplugin.la
endif

# build a Player plugin from libstage plus Player driver hooks.
libstageplugin_la_SOURCES = \
	p_driver.cc \
	p_driver.h \
	p_graphics3d.cc \
	p_laser.cc \
	p_position.cc \
	p_simulation.cc \
	p_sonar.cc \
	stg_time.cc \
	stg_time.h 

# TODO
#	p_gripper.cc 
#	p_power.cc 
#	p_graphics2d.cc
#	p_graphics3d.cc 
#	p_bumper.cc 
#	p_fiducial.cc 
#	p_ptz.cc 
#	p_wifi.cc 
#	p_speech.cc 
#	p_localize.cc 
#	p_map.cc 
#	p_blobfinder.cc 

libstageplugin_la_CPPFLAGS = $(AM_CPPFLAGS) @PLAYER_CFLAGS@
libstageplugin_la_LDFLAGS = -version-info 1:0:0 -rpath $(libdir) -L../libstage 
libstageplugin_la_LIBADD = ../libstage/libstage.la @PLAYER_LIBS@

# build a Player plugin from libstage plus Player driver hooks.
#libglutgraphics_la_SOURCES = glutgraphics.cc
#libglutgraphics_la_CPPFLAGS = $(AM_CPPFLAGS) @PLAYER_CFLAGS@
#libglutgraphics_la_LDFLAGS = -version-info 1:0:0 -rpath $(libdir) -framework GLUT
#libglutgraphics_la_LIBADD = libstage.la @PLAYER_LIBS@ 

# build a player client that tests the plugin
#ptest_DEPENDENCIES = 
#ptest_SOURCES = ptest.c
#ptest_CFLAGS = $(AM_CFLAGS) @PLAYER_CFLAGS@ 
#ptest_LDADD = @PLAYERC_LIBS@
