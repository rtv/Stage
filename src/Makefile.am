
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.29.4.20 2003-08-09 00:58:34 rtv Exp $

stage_DEPENDENCIES = libstage.a $(top_srcdir)/replace/libreplace.a

AM_CPPFLAGS = -Wall -I$(top_builddir)/replace @RTK_CFLAGS@

lib_LIBRARIES = libstage.a libstageworldfile.a 
bin_PROGRAMS = stage

# install these headers
include_HEADERS = stage.h

#
# stage
#
stage_SOURCES = \
	entity.cc \
	entity.hh \
	matrix.cc \
	matrix.hh \
	raytrace.cc \
	raytrace.hh \
	rtkgui.cc \
	rtkgui.hh \
	stage.h\
	world.cc \
	world.hh \
	main.cc 

stage_LDFLAGS =
stage_LDADD =  @RTK_LIBS@ -L. -lstage 
#-lefence

# these libraries are useful for clients such as Player's stage1p4 driver
libstage_a_SOURCES = stage.h stage.c
libstageworldfile_a_SOURCES = worldfile.cc worldfile.hh stage.h
