if WITH_GUI
RTKCPPFLAGS = -I$(top_srcdir)/rtk2 `gtk-config --cflags`
RTKLDADD    = -L$(top_builddir)/rtk2 -lrtk
RTKLDFLAGS  = `gtk-config --libs gtk gthread`
else
RTKCPPFLAGS =
RTKLDADD    =
RTKLDFLAGS  =
endif

if WITH_RTVG
RTVGCPPFLAGS = $(GNOME2_CFLAGS)
RTVGLDADD    =
RTVGLDFLAGS  = $(GNOME2_LIBS)
else
RTVGCPPFLAGS =
RTVGLDADD    =
RTVGLDFLAGS  =
endif


AM_CPPFLAGS = -Wall -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage $(RTKCPPFLAGS) $(RTVGCPPFLAGS)

bin_PROGRAMS = stage

stage_SOURCES = \
		bitmap.cc			\
		bitmap.hh			\
		boxobstacle.cc			\
		boxobstacle.hh			\
		broadcastdevice.cc		\
		broadcastdevice.hh		\
		client.cc			\
		colors.cc			\
		colors.hh			\
		descartesdevice.cc		\
		descartesdevice.hh		\
		entity.cc			\
		entity.hh			\
                gnomegui.cc                     \
		gpsdevice.cc			\
		gpsdevice.hh			\
		gripperdevice.cc		\
		gripperdevice.hh		\
		idardevice.cc			\
		idardevice.hh			\
		idarturretdevice.cc		\
		idarturretdevice.hh		\
		image.cc			\
		image.hh			\
		laserbeacon.cc			\
		laserbeacon.hh			\
		laserbeacondevice.cc		\
		laserbeacondevice.hh		\
		laserdevice.cc			\
		laserdevice.hh			\
		library.cc			\
		library.hh			\
		main.cc				\
		matrix.cc			\
		matrix.hh			\
		motedevice.cc			\
		motedevice.hh			\
		omnipositiondevice.cc		\
		omnipositiondevice.hh		\
		playerdevice.cc			\
		playerdevice.hh			\
		positiondevice.cc		\
		positiondevice.hh		\
		ptzdevice.cc			\
		ptzdevice.hh			\
		puck.cc				\
		puck.hh				\
		raytrace.cc			\
		raytrace.hh			\
                rtkgui.cc                       \
		rtp.cc				\
		rtp.h				\
		server.cc			\
		server.hh			\
		sonardevice.cc			\
		sonardevice.hh			\
		stage_types.hh			\
		stageio.cc			\
		truthdevice.cc			\
		truthdevice.hh			\
		visionbeacon.cc			\
		visionbeacon.hh			\
		visiondevice.cc			\
		visiondevice.hh			\
		world.cc			\
		world.hh			\
		worldfile.cc			\
		worldfile.hh 
                #bpsdevice.cc
                #entityfactory.cc 
             #bpsdevice.hh
             #miscdevice.hh


stage_LDADD = -lm -L@PLAYER_DIR@/lib -L@PLAYER_DIR@/server -lplayerqueue $(RTKLDADD) $(RTVGLDADD) @LIBOBJS@ 

stage_LDFLAGS =  $(RTKLDFLAGS) $(RTVGLDFLAGS)
