
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.3 2002-10-28 05:31:39 gerkey Exp $

AM_CPPFLAGS = 	-I$(top_srcdir)/src \
		-I$(top_srcdir)/src/models \
		-I@PLAYER_DIR@/include \
		-I$(top_srcdir)/rtk2 `gtk-config --cflags`

noinst_LIBRARIES = librtkstage.a

librtkstage_a_SOURCES = rtkgui.cc

