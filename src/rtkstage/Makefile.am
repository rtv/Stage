
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.2 2002-10-28 02:41:17 rtv Exp $

AM_CPPFLAGS = 	-I$(top_srcdir)/src \
		-I$(top_srcdir)/src/models \
		-I@PLAYER_DIR@/include \
		-I$(top_srcdir)/rtk2 `gtk-config --cflags`

lib_LIBRARIES = librtkstage.a

noinst_LIBRARIES = librtkstage.a

librtkstage_a_SOURCES = rtkgui.cc

