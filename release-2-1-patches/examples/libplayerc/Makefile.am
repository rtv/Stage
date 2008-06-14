# $Id: Makefile.am,v 1.3 2006-08-02 21:48:07 pooya Exp $

# system-wide compile flags - target-specific flags are added to each target below
AM_CFLAGS =   -Wall -g -I. @PLAYERC_CFLAGS@
AM_CPPFLAGS = -Wall -g -I. @PLAYERC_CFLAGS@

# if Player is present, build the plugin and the test program
if HAVE_PLAYERC
  bin_PROGRAMS = ptest audio
endif

ptest_SOURCES = ptest.c
ptest_DEPENDENCIES =
ptest_LDADD = @PLAYERC_LIBS@

audio_SOURCES = audio.c
audio_DEPENDENCIES =
audio_LDADD = @PLAYERC_LIBS@
