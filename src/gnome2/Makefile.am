# Automake script to build Stage GNOME2 GUI Makefile
# RTV 20021026
# $Id: Makefile.am,v 1.3 2002-10-28 02:41:17 rtv Exp $

lib_LIBRARIES = libgnomestage.a 

noinst_LIBRARIES = libgnomestage.a

AM_CPPFLAGS = 	-I$(top_srcdir)/src \
		-I$(top_srcdir)/src/models \
		-I@PLAYER_DIR@/include \
		`pkg-config --cflags libgnomecanvas-2.0 libgnomeui-2.0`

libgnomestage_a_SOURCES = gnomegui.cc gnomedevices.cc

