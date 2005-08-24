lib_LTLIBRARIES = libmyref.la libtransref.la

AM_CPPFLAGS = \
	-Wall \
	-I. -I$(top_builddir) -I$(top_builddir)/src -I$(top_builddir)/replace \
	@PLAYER_CFLAGS@ \
	@GTK_CFLAGS@

# This is important because people's referees need to know about ZooReferee.
# But isn't this already... in there someplace, by the time the ref gets
# loaded?  Oh well.
LIBS = -L$(prefix)/lib -lstageplugin

libmyref_la_SOURCES = myref.cc

libtransref_la_SOURCES = transref.cc
libtransref_la_LIBADD = @PLAYER_LIBS@
