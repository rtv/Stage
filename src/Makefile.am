
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.96 2005-06-02 09:43:28 rtv Exp $

# install the header file for the Stage library
include_HEADERS = stage.h 

AM_CFLAGS = -Wall -I. -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@
AM_CPPFLAGS = -Wall -I. -I$(top_builddir)/replace @PLAYER_CFLAGS@ @GTK_CFLAGS@

# build the stage library and the stage plugin
lib_LTLIBRARIES = libstage.la libstageplugin.la

# build a stand-alone test binary using the stage library
bin_PROGRAMS = stest


# The stage library contains everything you need to build a robot
# simulation, including graphics and a world description file
# parser. It does NOT contain a Player interface, so it can not be
# used as a player plugin. See libstageplugin (below) for good Player
# satisfaction.
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
	model_gripper.c \
	model_laser.c \
	model_position.c \
	model_ranger.c \
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

libstage_la_LDFLAGS = -version-info 1:0:0 
libstage_la_LIBADD = @GTK_LIBS@

# build a stand-alone binary using libstage for testing purposes
stest_DEPENDENCIES = libstage.la
stest_SOURCES = stest.cc
stest_LDADD = libstage.la

# build a Player plugin from libstage plus Player driver hooks.
libstageplugin_la_SOURCES = player_driver.cc player_interfaces.cc 
libstageplugin_la_LDFLAGS = -export-dynamic -version-info 1:0:0 -L.libs -lstage

