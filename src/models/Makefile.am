#
# Automake config to build the device models
#
# RTV 20021026
# $Id
#
# we need extra flags as long as RTK code is embedded in the main simulator source
if WITH_RTK
RTKCPPFLAGS = -I@RTK_DIR@/include `gtk-config --cflags`
endif

AM_CPPFLAGS = 	-Wall -I$(top_builddir)/src  -I$(top_builddir)/replace $(RTKCPPFLAGS)

noinst_LIBRARIES = libmodels.a 

libmodels_a_SOURCES = \
	bitmap.cc \   #remove this once we have some files in here
	bitmap.hh

#	box.cc \
#	box.hh \
#	positiondevice.cc \
#	positiondevice.hh
#                   broadcastdevice.cc \
#                   broadcastdevice.hh \
#                   bumperdevice.cc \
#                   bumperdevice.hh \
#                   fiducialfinderdevice.cc \
#                   fiducialfinderdevice.hh \
#                   gpsdevice.cc \
#                   gpsdevice.hh \
#                   gripperdevice.cc \
#                   gripperdevice.hh \
#                   idardevice.cc \
#                   idardevice.hh \
#                   idarturretdevice.cc \
#                   idarturretdevice.hh \
#                   image.cc \
#                   image.hh \
#                   laserdevice.cc \
#                   laserdevice.hh \
#                   motedevice.cc \
#                   motedevice.hh \
#                   powerdevice.cc \
#                   powerdevice.hh \
#                   ptzdevice.cc \
#                   ptzdevice.hh \
#                   puck.cc \
#                   puck.hh \
#                   sonardevice.cc \
#                   sonardevice.hh \
#                   truthdevice.cc \
#                   truthdevice.hh \
#                   visiondevice.cc \
#                   visiondevice.hh \
#		   regularmcldevice.hh \
#		   regularmcldevice.cc


# currently broken
# bpsdevice.cc 