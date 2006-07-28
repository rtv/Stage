# $Id: Makefile.am,v 1.1 2006-07-28 22:12:43 pooya Exp $

# system-wide compile flags - target-specific flags are added to each target below
#AM_CFLAGS =   -Wall -I. -I$(top_srcdir)/replace @GTK_CFLAGS@  -std=c99 
#AM_CPPFLAGS = -Wall -I. -I$(top_srcdir)/replace @GTK_CFLAGS@
AM_CFLAGS =   -Wall -g -I. 
AM_CPPFLAGS = -Wall -g -I. 

# if Player is present, build the plugin and the test program
if HAVE_PLAYERC
 bin_PROGRAMS = ptest
endif

ptest_DEPENDENCIES = 
ptest_SOURCES = ptest.c
ptest_CFLAGS = $(AM_CFLAGS) @PLAYERC_CFLAGS@ 
ptest_LDADD = @PLAYERC_LIBS@
