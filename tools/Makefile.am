toolsdir = $(prefix)/bin

tools_PROGRAMS = autoplace

AM_CPPFLAGS = @PLAYER_CFLAGS@
autoplace_SOURCES = autoplace.cc
autoplace_LDADD = @PLAYER_LIBS@

tools_SCRIPTS = worldfileconv.tcl \
                pause

EXTRA_DIST =  worldfileconv.tcl \
              pause
