
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.1.2.1 2003-02-05 04:03:21 rtv Exp $

if WITH_RTK
noinst_LIBRARIES = librtkgui.a
endif

AM_CPPFLAGS = 	-I$(top_srcdir)/src -I$(top_srcdir)/replace -I$(top_srcdir)/src/models -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage -I@PLAYER_DIR@/client_libs/libplayerpacket -I@RTK_DIR@/include `gtk-config --cflags`

librtkgui_a_SOURCES = rtkgui.cc
