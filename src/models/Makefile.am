#
# Automake config to build the device models
#
# RTV 20021026
# $Id
#

# we need this while rtk code is still in the device files
if WITH_RTK
RTKCPPFLAGS = -I@RTK_DIR@/include `gtk-config --cflags`
endif


AM_CPPFLAGS = 	-Wall -I$(top_srcdir)/src -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage  -I@PLAYER_DIR@/client_libs/libplayerpacket $(RTKCPPFLAGS)

noinst_LIBRARIES = libstgmodels.a 

libstgmodels_a_SOURCES = \
                   bitmap.cc \
                   bitmap.hh \
                   box.cc \
                   box.hh \
                   broadcastdevice.cc \
                   broadcastdevice.hh \
                   bumperdevice.cc \
                   bumperdevice.hh \
                   fiducialfinderdevice.cc \
                   fiducialfinderdevice.hh \
                   gpsdevice.cc \
                   gpsdevice.hh \
                   gripperdevice.cc \
                   gripperdevice.hh \
                   idardevice.cc \
                   idardevice.hh \
                   idarturretdevice.cc \
                   idarturretdevice.hh \
                   image.cc \
                   image.hh \
                   laserdevice.cc \
                   laserdevice.hh \
                   motedevice.cc \
                   motedevice.hh \
                   positiondevice.cc \
                   positiondevice.hh \
                   powerdevice.cc \
                   powerdevice.hh \
                   ptzdevice.cc \
                   ptzdevice.hh \
                   puck.cc \
                   puck.hh \
                   sonardevice.cc \
                   sonardevice.hh \
                   truthdevice.cc \
                   truthdevice.hh \
                   visiondevice.cc \
                   visiondevice.hh \
		   regularmcldevice.hh \
		   regularmcldevice.cc \
		   regularmcldevice_world.cc \
		   regularmcldevice_sensor.cc \
		   regularmcldevice_action.cc \
		   regularmcldevice_clustering.cc

# currently broken
# bpsdevice.cc 

# REMOVE
#                   positiondevice.cc \
#                   positiondevice.hh \

