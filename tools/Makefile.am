toolsdir = $(prefix)/tools

tools_PROGRAMS = autoplace

AM_CPPFLAGS = @PLAYER_CPPFLAGS@
autoplace_SOURCES = autoplace.cc
autoplace_LDADD = @PLAYER_LDADD@

tools_SCRIPTS = worldfileconv.tcl \
                pause

EXTRA_DIST =  worldfileconv.tcl \
              pause
