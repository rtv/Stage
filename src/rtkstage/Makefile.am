
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.1 2002-10-27 23:12:27 rtv Exp $

AM_CPPFLAGS = 	-I$(top_srcdir)/src \
		-I$(top_srcdir)/src/models \
		-I@PLAYER_DIR@/include \
		-I$(top_srcdir)/rtk2 `gtk-config --cflags`



noinst_LIBRARIES = librtkstage.a

lib_LIBRARIES = librtkstage.a

librtkstage_a_SOURCES = rtkgui.cc

