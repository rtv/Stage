
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.29.4.18 2003-02-24 04:47:11 rtv Exp $

SUBDIRS = sio rtkgui models
# gnome2

# we need extra flags as long as RTK code is embedded in the main
# simulator source
if WITH_RTK
RTKCPPFLAGS = -I@RTK_DIR@/include `gtk-config --cflags`
endif

stage_DEPENDENCIES = models/libmodels.a sio/libsio.a $(top_srcdir)/replace/libreplace.a  @GUI_DIR@/@GUI_LIB@ 

AM_CPPFLAGS = -Wall -Isio -I$(top_builddir)/replace $(RTKCPPFLAGS)

bin_PROGRAMS = stage stest

stage_SOURCES = \
	colors.cc \
	colors.hh \
	entity.cc \
	entity.hh \
	gui.h \
	library.cc \
	library.hh \
	main.cc \
	matrix.cc \
	matrix.hh \
	property.cc \
	raytrace.cc \
	raytrace.hh \
	raytrace.hh \
	root.cc \
	root.hh \
	stage.h

stage_LDFLAGS = 
stage_LDADD = @GUI_LINK@ -lm -Lmodels -lmodels -Lsio -lsio -L$(top_builddir)/replace -lreplace 

# build Stage's test program - a simple client
#	
stest_SOURCES = stest.c 
stest_DEPENDENCIES = sio/libsio.a $(top_srcdir)/replace/libreplace.a
stest_LDADD = -lpnm -lm -Lsio -lsio -L$(top_builddir)/replace -lreplace 
stest_CFLAGS = -g2