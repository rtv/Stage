
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.9 2008-02-24 20:46:13 rtv Exp $

# install the header file for the Stage library
include_HEADERS = stage.hh 

# install the color database
pkgdata_DATA = rgb.txt

RGB=$(datadir)/@PACKAGE@/rgb.txt

# system-wide compile flags - target-specific flags are added to each target below
AM_CPPFLAGS = -Wall -I. -I$(top_srcdir)/replace  @PIXBUF_CFLAGS@ @FLTK_CFLAGS@ -DRGBFILE=\"$(RGB)\"

# build the stage library and program
lib_LTLIBRARIES = libstage.la 
bin_PROGRAMS = stage

libstage_la_SOURCES =  \
	../config.h \
	ancestor.cc \
	block.cc \
	canvas.cc \
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

libstage_la_LDFLAGS = -version-info 3:1:0 -rpath $(libdir) 
libstage_la_LIBADD = @PIXBUF_LIBS@ @FLTK_LIBS@  -lltdl

if DARWIN
libstage_la_LIBADD += 
else
libstage_la_LIBADD += -lGLU
endif


# STAGE
stage_DEPENDENCIES = libstage.la
stage_SOURCES = main.cc
stage_LDADD = libstage.la

