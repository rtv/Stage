
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.8 2002-11-19 04:27:19 rtv Exp $

if WITH_RTK
noinst_LIBRARIES = librtkstage.a
endif

AM_CPPFLAGS = 	-I$(top_srcdir)/src -I$(top_srcdir)/src/models -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage -I@PLAYER_DIR@/client_libs/libplayerpacket -I@RTK_DIR@/include `gtk-config --cflags`

librtkstage_a_SOURCES = rtkgui.cc
