
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.4 2002-10-28 05:48:16 gerkey Exp $

AM_CPPFLAGS = 	-I$(top_srcdir)/src \
		-I$(top_srcdir)/src/models \
		-I@PLAYER_DIR@/include \
		-I@PLAYER_DIR@/server \
		-I@PLAYER_DIR@/server/drivers/stage \
		-I$(top_srcdir)/rtk2 `gtk-config --cflags`

noinst_LIBRARIES = librtkstage.a

librtkstage_a_SOURCES = rtkgui.cc

