bin_PROGRAMS = test

AM_CPPFLAGS = -Wall -I$(top_srcdir)/src -I$(top_srcdir)/src/stageio -I$(top_srcdir)/replace 

test_SOURCES = test.c

test_LDADD =  -L$(top_builddir)/src/stageio -lstageio -L$(top_builddir)/replace -lreplace