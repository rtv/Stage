
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.5 2002-10-31 07:48:35 gerkey Exp $

if HAVE_GTK
if WITH_RTK_GUI
noinst_LIBRARIES = librtkstage.a
endif
endif

AM_CPPFLAGS = 	-I$(top_srcdir)/src -I$(top_srcdir)/src/models -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage -I$(top_srcdir)/rtk2 `gtk-config --cflags`


librtkstage_a_SOURCES = rtkgui.cc
