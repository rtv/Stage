#
# Automake config to build the device models
#
# RTV 20021026
# $Id
#

# we need this while rtk code is still in the device files
if WITH_GUI
RTKCPPFLAGS = -I$(top_srcdir)/rtk2 `gtk-config --cflags`
endif


AM_CPPFLAGS = 	-Wall \
		-I$(top_srcdir)/src \
		-I@PLAYER_DIR@/include \
		-I@PLAYER_DIR@/server \
		-I@PLAYER_DIR@/server/drivers/stage \
		$(RTKCPPFLAGS)

lib_LIBRARIES = libstgmodels.a 

libstgmodels_a_SOURCES = \
			bitmap.cc \
			box.cc \
			broadcastdevice.cc \
			descartesdevice.cc \
			gpsdevice.cc \
			gripperdevice.cc \
			idardevice.cc \
			idarturretdevice.cc \
			image.cc \
			laserbeacon.cc \
			laserbeacondevice.cc \
			laserdevice.cc \
			motedevice.cc \
			omnipositiondevice.cc \
			positiondevice.cc \
			powerdevice.cc \
			ptzdevice.cc \
			puck.cc \
			sonardevice.cc \
			truthdevice.cc \
			visionbeacon.cc \
			visiondevice.cc

# currently broken
# bpsdevice.cc 

# deprecated - remove
# miscdevice.cc 

