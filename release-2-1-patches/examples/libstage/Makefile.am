# $Id: Makefile.am,v 1.3 2006-08-03 01:11:35 pooya Exp $

LIBSTAGE_CFLAGS = -I$(top_srcdir)/src @GTK_CFLAGS@
LIBSTAGE_LIBS = -L$(top_builddir)/src -lstage @GTK_LIBS@

# system-wide compile flags - target-specific flags are added to each target below
AM_CFLAGS =   -Wall -g -I. $(LIBSTAGE_CFLAGS)
AM_CPPFLAGS = -Wall -g -I. $(LIBSTAGE_CFLAGS)

# build stest, a stand-alone test binary using the stage library, 
bin_PROGRAMS = stest 

# build a stand-alone test program using libstage
stest_SOURCES = stest.c
stest_DEPENDENCIES =
stest_LDADD = $(LIBSTAGE_LIBS) 
