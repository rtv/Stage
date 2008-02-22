# $Id: Makefile.am,v 1.1 2008-02-22 21:20:47 rtv Exp $

# system-wide compile flags - target-specific flags are added to each target below
AM_CPPFLAGS = -Wall -I. -I../../libstage -I$(top_srcdir)/replace  @PIXBUF_CFLAGS@ @FLTK_CFLAGS@ -DRGBFILE=\"$(RGB)\"


lib_LTLIBRARIES = wander.la

# WANDER CONTROLLER
wander_la_SOURCES = wander.cc ../../libstage/stage.hh
wander_la_LDFLAGS = -module 

