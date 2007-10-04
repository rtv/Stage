
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.1.2.1 2007-10-04 05:32:18 rtv Exp $

# also build Zoo referees
#SUBDIRS = referees

# install the header file for the Stage library
include_HEADERS = stage.hh 

# install the color database
pkgdata_DATA = rgb.txt

CC=g++

RGB=$(datadir)/@PACKAGE@/rgb.txt

# system-wide compile flags - target-specific flags are added to each target below
#AM_CFLAGS =   -Wall -I. -I$(top_srcdir)/replace @GUI_CFLAGS@ -std=c99  
AM_CPPFLAGS = -Wall -I. -I$(top_srcdir)/replace  @GUI_CFLAGS@ -DRGBFILE="$(RGB)"

# build the stage library
lib_LTLIBRARIES = libstage.la 

# TODO put this back in before release
# build stest, a stand-alone test binary using the stage library, 
bin_PROGRAMS = stest 

# The stage library contains everything you need to build a robot
# simulation, including graphics and a world description file
# parser. It does NOT contain a Player interface, so it can not be
# used as a player plugin. See libstageplugin (below) for good Player
# satisfaction.
#
libstage_la_SOURCES =  \
	../config.h \
	ancestor.cc \
	block.cc \
	cell.cc \
	colors.h \
	gl.c \
	glcolorstack.cc \
	logo.cc \
	model.cc \
	model_callbacks.cc \
	model_load.cc \
	model_props.cc \
	stage.c \
	stage.hh \
	typetable.cc \
	world.cc \
	worldfile.cc \
	worldfile.hh \
	worldgtk.cc 


# TODO
#	gui_gtk_prefs.c
#	model_blobfinder.c 
#	model_bumper.c 
#	model_fiducial.c 
#	model_fiducial.hh
#	model_gripper.c 
#	model_laser.cc
#	model_laser.hh 
#	model_position.cc 
#	model_position.hh 
#	model_ptz.c 
#	model_ranger.cc 
#	model_ranger.hh 
#	model_wifi.cc 
# 	model_power.c # needs lots of work


libstage_la_LDFLAGS = -version-info 3:1:0 -rpath $(libdir) 
libstage_la_LIBADD = @GUI_LIBS@

# TODO fix these and move them to a separate tree?
#zoo.c 
#zoo_driver.cc 
#zoo_species.cc 
#zoo_controller.cc 
#zoo_referee.cc 

# build a stand-alone test program using libstage
stest_DEPENDENCIES = libstage.la
stest_SOURCES = stest.cc

stest_LDADD = libstage.la @GUI_LIBS@  
#stest_LDFLAGS = 
