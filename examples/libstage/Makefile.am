# $Id: Makefile.am,v 1.1 2006-07-28 22:12:43 pooya Exp $

LIBSTAGE_CFLAGS = -I$(top_srcdir)/src
LIBSTAGE_LIB_DIRS = -L$(top_builddir)/src
LIBSTAGE_DEPS = $(top_builddir)/src/libstage.la

# system-wide compile flags - target-specific flags are added to each target below
AM_CFLAGS =   -Wall -g -I. -I$(top_srcdir)/replace $(LIBSTAGE_CFLAGS) @GTK_CFLAGS@  -std=c99 
AM_CPPFLAGS = -Wall -g -I. -I$(top_srcdir)/replace $(LIBSTAGE_CFLAGS) @GTK_CFLAGS@

# build stest, a stand-alone test binary using the stage library, 
bin_PROGRAMS = stest 

# build a stand-alone test program using libstage
stest_SOURCES = stest.c
stest_LDADD = $(LIBSTAGE_LIB_DIRS) -lstage
stest_DEPENDENCIES = $(LIBSTAGE_DEPS)
