
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.26 2004-09-11 23:04:00 rtv Exp $

lib_LIBRARIES = libmodels.a

libmodels_a_DEPENDENCIES = $(top_srcdir)/replace/libreplace.a

AM_CFLAGS = -I.. -Wall -I$(top_builddir)/replace @RTK_CFLAGS@

libmodels_a_SOURCES = \
	model_blobfinder.c \
	model_energy.c \
	model_fiducial.c \
	model_laser.c \
	model_pose.c \
	model_position.c \
	model_props.c \
	model_ranger.c \
	model_render.c \
	model_test.c

