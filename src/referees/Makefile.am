lib_LTLIBRARIES = libmyref.la libtransref1.la

AM_CPPFLAGS = \
	-Wall \
	-I. -I$(top_builddir) -I$(top_builddir)/src -I$(top_builddir)/replace \
	@PLAYER_CFLAGS@ \
	@GTK_CFLAGS@
LIBADD = @GTK_LIBS@ @STAGE_LIBS@ -lstageplugin
LDFLAGS = -export-dynamic

libmyref_la_SOURCES = myref.cc
libtransref1_la_SOURCES = transref1.cc
