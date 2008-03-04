# $Id: Makefile.am,v 1.3 2008-03-04 02:09:56 rtv Exp $

# system-wide compile flags - target-specific flags are added to each target below
AM_CPPFLAGS = -Wall -I. -I../../libstage -I$(top_srcdir)/replace  @PIXBUF_CFLAGS@ @FLTK_CFLAGS@ -DRGBFILE=\"$(RGB)\"


lib_LTLIBRARIES = wander.la fasr.la source.la sink.la lasernoise.la

# WANDER CONTROLLER
wander_la_SOURCES = wander.cc ../../libstage/stage.hh
wander_la_LDFLAGS = -module 

# FASR controller
fasr_la_SOURCES = fasr.cc ../../libstage/stage.hh
fasr_la_LDFLAGS = -module 

# resource source
source_la_SOURCES = source.cc ../../libstage/stage.hh
source_la_LDFLAGS = -module 

# resource sink
sink_la_SOURCES = sink.cc ../../libstage/stage.hh
sink_la_LDFLAGS = -module 

# laser noise
lasernoise_la_SOURCES = lasernoise.cc ../../libstage/stage.hh
lasernoise_la_LDFLAGS = -module
