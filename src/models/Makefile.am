#
# Automake config to build the device models
#
# RTV 20021026
# $Id
#

# we need this while rtk code is still in the device files
if WITH_GUI
RTKCPPFLAGS = -I$(top_srcdir)/src/rtk2 `gtk-config --cflags`
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
			image.cc \
			box.cc \
			idardevice.cc \
			motedevice.cc \
			sonardevice.cc \
			idarturretdevice.cc \
			omnipositiondevice.cc \
			truthdevice.cc \
			broadcastdevice.cc \
			laserbeacon.cc \
			positiondevice.cc \
			visionbeacon.cc \
			descartesdevice.cc \
			laserbeacondevice.cc \
			powerdevice.cc \
			visiondevice.cc \
			gpsdevice.cc \
			laserdevice.cc \
			ptzdevice.cc \
			gripperdevice.cc \
			puck.cc 

# currently broken
# bpsdevice.cc 

# deprecated - remove
# miscdevice.cc 

