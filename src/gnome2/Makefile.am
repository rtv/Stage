# Automake script to build Stage GNOME2 GUI Makefile
# RTV 20021026
# $Id: Makefile.am,v 1.5 2002-10-31 07:48:35 gerkey Exp $

if WITH_GNOME2
noinst_LIBRARIES = libgnomestage.a
endif

AM_CPPFLAGS = 	-I$(top_srcdir)/src -I$(top_srcdir)/src/models -I@PLAYER_DIR@/include `pkg-config --cflags libgnomecanvas-2.0 libgnomeui-2.0`

libgnomestage_a_SOURCES = gnomegui.cc gnomegui.hh gnomedevices.cc

