
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.6 2002-11-01 19:12:32 rtv Exp $

if HAVE_GTK
if WITH_RTK_GUI
noinst_LIBRARIES = librtkstage.a
endif
endif

AM_CPPFLAGS = 	-I$(top_srcdir)/src -I$(top_srcdir)/src/models -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage -I$(top_srcdir)/rtk2 `gtk-config --cflags`


librtkstage_a_SOURCES = rtkgui.cc

librtkstage_a_DEPENDENCIES = $(top_srcdir)/rtk2/librtk.a
