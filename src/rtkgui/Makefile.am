
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.1.2.2 2003-02-07 07:47:01 rtv Exp $

if WITH_RTK
noinst_LIBRARIES = librtkgui.a
endif

AM_CPPFLAGS = 	-I$(top_srcdir)/src -I$(top_srcdir)/replace -I$(top_srcdir)/src/models -I@RTK_DIR@/include `gtk-config --cflags`

librtkgui_a_SOURCES = \
	rtkgui.cc \
	rtkgui_entity.cc
