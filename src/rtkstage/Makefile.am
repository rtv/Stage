
# Automake config to build the rtkstage gui library Makefile
# $Id: Makefile.am,v 1.10 2003-05-24 21:08:34 gerkey Exp $

if WITH_RTK
noinst_LIBRARIES = librtkstage.a
endif

AM_CPPFLAGS = 	-I$(top_srcdir)/src -I$(top_srcdir)/replace -I$(top_srcdir)/src/models @PLAYER_CPPFLAGS@ @RTK_CPPFLAGS@

librtkstage_a_SOURCES = rtkgui.cc
