toolsdir = $(prefix)/tools

tools_PROGRAMS = autoplace

CPPFLAGS = -I@PLAYER_DIR@/include -I@PLAYER_DIR@/client_libs/c -I@PLAYER_DIR@/client_libs/c++ -I@PLAYER_DIR@/server
autoplace_SOURCES = autoplace.cc
autoplace_LDADD = -L@PLAYER_DIR@/lib -L@PLAYER_DIR@/client_libs/c++ -lplayerclient



tools_SCRIPTS = worldfileconv.tcl \
                pause

EXTRA_DIST =  worldfileconv.tcl \
              pause
