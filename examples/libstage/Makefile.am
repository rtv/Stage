# $Id: Makefile.am,v 1.4 2008-01-15 01:25:42 rtv Exp $

LIBSTAGE_CFLAGS = -I$(top_srcdir)/libstage @PIXBUF_CFLAGS@ @FLTK_CFLAGS@
LIBSTAGE_LIBS = -L$(top_builddir)/libstage -lstage @PIXBUF_LIBS@ @FLTK_LIBS@

# system-wide compile flags - target-specific flags are added to each target below
AM_CFLAGS =   -Wall -g -I. $(LIBSTAGE_CFLAGS)
AM_CPPFLAGS = -Wall -g -I. $(LIBSTAGE_CFLAGS)

# build stest, a stand-alone test binary using the stage library, 
bin_PROGRAMS = stest 

# build a stand-alone test program using libstage
stest_SOURCES = stest.cc
stest_DEPENDENCIES =
stest_LDADD = $(LIBSTAGE_LIBS) 
