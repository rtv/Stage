# Automake script to build Stage GNOME2 GUI Makefile
# RTV 20021026
# $Id

lib_LIBRARIES = libgnomestage.a 

AM_CPPFLAGS = 	-I$(top_srcdir)/src \
		-I$(top_srcdir)/src/models \
		-I@PLAYER_DIR@/include \
		`pkg-config --cflags libgnomecanvas-2.0 libgnomeui-2.0`

libgnomestage_a_SOURCES = gnomegui.cc gnomedevices.cc

