
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.4 2008-02-20 06:03:58 rtv Exp $

# install the header file for the Stage library
include_HEADERS = stage.hh 

# install the color database
pkgdata_DATA = rgb.txt

RGB=$(datadir)/@PACKAGE@/rgb.txt

# system-wide compile flags - target-specific flags are added to each target below
AM_CPPFLAGS = -Wall -I. -I$(top_srcdir)/replace  @PIXBUF_CFLAGS@ @FLTK_CFLAGS@ -DRGBFILE=\"$(RGB)\"

# build the stage library
lib_LTLIBRARIES = libstage.la ctrl.la

# The stage library 
libstage_la_SOURCES =  \
	../config.h \
	ancestor.cc \
	block.cc \
	canvas.cc \
	colors.h \
	gl.cc \
	glcolorstack.cc \
	model.cc \
	model_callbacks.cc \
	model_fiducial.cc \
	model_laser.cc \
	model_load.cc \
	model_position.cc \
	model_ranger.cc \
	model_blobfinder.cc \
	model_props.cc \
	stage.cc \
	stage.hh \
	stage_internal.hh \
	typetable.cc \
	world.cc \
	worldfile.cc \
	worldfile.hh \
	worldgui.cc 

#logo.cc 
libstage_la_LDFLAGS = -version-info 3:1:0 -rpath $(libdir) 
libstage_la_LIBADD = @PIXBUF_LIBS@ @FLTK_LIBS@  -lltdl

# build stest, a stand-alone test binary using the stage library, 
#bin_PROGRAMS = stest stage
#stest_DEPENDENCIES = libstage.la
#stest_SOURCES = stest.cc
#stest_LDADD = libstage.la

bin_PROGRAMS = stage

stage_DEPENDENCIES = libstage.la
stage_SOURCES = main.cc
stage_LDADD = libstage.la

ctrl_la_SOURCES = ctrl.cc stage.hh
ctrl_la_LDFLAGS = -module 

