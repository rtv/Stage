toolsdir = $(prefix)/tools

tools_PROGRAMS = autoplace

CPPFLAGS = -I@PLAYER_DIR@/include -I@PLAYER_DIR@/client_libs/c -I@PLAYER_DIR@/client_libs/c++ -I@PLAYER_DIR@/server
autoplace_SOURCES = autoplace.cc
autoplace_LDADD = -L@PLAYER_DIR@/lib -L@PLAYER_DIR@/client_libs/c++ -lplayerclient

tools_SCRIPTS = dstage \
                killdist.tcl \
                push \
                run \
                slay \
                startdist.tcl \
                worldfileconv.tcl

EXTRA_DIST =    dstage \
                killdist.tcl \
                push \
                run \
                slay \
                startdist.tcl \
                worldfileconv.tcl
