AM_CPPFLAGS = -Wall -I$(top_srcdir)/include -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage -I$(top_srcdir)/rtk2 `gtk-config --cflags`

bin_PROGRAMS = stage

stage_SOURCES = boxobstacle.cc \
                broadcastdevice.cc \
                client.cc \
                colors.cc \
                descartesdevice.cc \
                entity.cc \
                entityfactory.cc \
                fixedobstacle.cc \
                gpsdevice.cc \
                gripperdevice.cc \
                idarturretdevice.cc \
                image.cc \
                irdevice.cc \
                laserbeacon.cc \
                laserbeacondevice.cc \
                laserdevice.cc \
                main.cc \
                matrix.cc \
                motedevice.cc \
                omnipositiondevice.cc \
                playerdevice.cc \
                positiondevice.cc \
                ptzdevice.cc \
                puck.cc \
                raytrace.cc \
                rtp.cc \
                server.cc \
                sonardevice.cc \
                stageio.cc \
                truthdevice.cc \
                visionbeacon.cc \
                visiondevice.cc \
                world.cc \
                worldfile.cc
                #bpsdevice.cc

stage_LDADD = -lm -L@PLAYER_DIR@/lib -L@PLAYER_DIR@/server -lplayerqueue -lpthread -L$(top_builddir)/rtk2 -lrtk

stage_LDFLAGS = `gtk-config --libs gtk gthread`
