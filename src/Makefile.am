
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.57 2004-08-27 23:59:09 rtv Exp $

stage_DEPENDENCIES = libstage.a $(top_srcdir)/replace/libreplace.a

AM_CFLAGS = -Wall -I$(top_builddir)/replace @RTK_CFLAGS@
AM_CPPFLAGS = -Wall -I$(top_builddir)/replace @RTK_CFLAGS@


lib_LIBRARIES = libstage.a

bin_PROGRAMS = stage 
#stest

# install these headers
include_HEADERS = stage.h

#
# stage program
#
stage_SOURCES = \
	connection.c \
	connection.h \
	gui.c \
	gui.h \
	gui_menus.c \
	library.c \
	main.c \
	matrix.c \
	matrix.h \
	model.c \
	model.h \
	model_energy.c \
	model_fiducial.c \
	model_laser.c \
	model_pose.c \
	model_position.c \
	model_props.c \
	model_render.c \
	model_test.c \
	raytrace.c \
	raytrace.h \
	server.c \
	server.h \
	stage.c \
	stage.h \
	subscription.c \
	subscription.h \
	world.c \
	world.h

#	model_blobfinder.c 
#	model_ranger.c 


stage_LDFLAGS =
stage_LDADD =  -L. @RTK_LIBS@ 

#
# test program
#
#stest_SOURCES = test2.cc worldfile.cc worldfile.hh

#stest_LDFLAGS =
#stest_LDADD = -L. @RTK_LIBS@ -lstage

#
# libstage.a
# this library is useful for clients such as Player's stage1p4 driver
#
libstage_a_SOURCES = stage.h stage.c stageclient.c bitmap.c token.c  worldfile.cc worldfile.hh stagecpp.cc