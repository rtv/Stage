
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.29.4.23 2003-08-19 00:57:19 rtv Exp $

stage_DEPENDENCIES = libstage.a $(top_srcdir)/replace/libreplace.a

AM_CPPFLAGS = -Wall -I$(top_builddir)/replace @RTK_CFLAGS@

lib_LIBRARIES = libstage.a libstagecpp.a 
bin_PROGRAMS = stage

# install these headers
include_HEADERS = stage.h stagecpp.hh

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
stage_LDADD =  @RTK_LIBS@  @LIBJPEG@ -L. -lstage 
#-lefence

# these libraries are useful for clients such as Player's stage1p4 driver
# you should use either the C libray OR the C++ library
# the C library has all the Stage server IO stuff
libstage_a_SOURCES = stage.h stage.c
# the C++ library has the same, plus some extra stuff for handling worldfiles  
libstagecpp_a_SOURCES = stage.h stage.c stagecpp.cc stagecpp.hh
