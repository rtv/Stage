
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.7.2.1.2.1 2004-02-26 16:26:56 gerkey Exp $

if WITH_RTK
noinst_LIBRARIES = librtkstage.a
endif

AM_CPPFLAGS = 	-I$(top_srcdir)/src -I$(top_srcdir)/replace -I$(top_srcdir)/src/models @PLAYER_CPPFLAGS@ @RTK_CFLAGS@

librtkstage_a_SOURCES = rtkgui.cc
