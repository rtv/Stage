
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.1.2.3 2003-02-08 01:20:37 rtv Exp $

if WITH_RTK
noinst_LIBRARIES = librtkgui.a
endif

AM_CPPFLAGS = 	-I$(top_srcdir)/src -I$(top_srcdir)/src/models -I@RTK_DIR@/include `gtk-config --cflags` -I$(top_srcdir)/replace 

librtkgui_a_SOURCES = \
	rtkgui.cc \
	rtkgui_entity.cc \
	rtkgui_sonar.cc

