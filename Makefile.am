SUBDIRS = rtk2 src worlds tools

EXTRA_DIST = include/boxobstacle.hh \
             include/broadcastdevice.hh \
             include/colors.hh \
             include/descartesdevice.hh \
             include/entityfactory.hh \
             include/entity.hh \
             include/fixedobstacle.hh \
             include/gpsdevice.hh \
             include/gripperdevice.hh \
             include/guiexport.hh \
             include/idarturretdevice.hh \
             include/image.hh \
             include/irdevice.hh \
             include/laserbeacondevice.hh \
             include/laserbeacon.hh \
             include/laserdevice.hh \
             include/matrix.hh \
             include/motedevice.hh \
             include/occupancydevice.hh \
             include/omnipositiondevice.hh \
             include/playerdevice.hh \
             include/positiondevice.hh \
             include/ptzdevice.hh \
             include/puck.hh \
             include/raytrace.hh \
             include/rtp.h \
             include/server.hh \
             include/sonardevice.hh \
             include/stage_types.hh \
             include/truthdevice.hh \
             include/visionbeacon.hh \
             include/visiondevice.hh \
             include/worldfile.hh \
             include/world.hh
             #include/bpsdevice.hh
             #include/miscdevice.hh

dist-hook:
	mkdir -p $(top_distdir)/doc
	cd $(srcdir)/doc && make ps
	cp -p $(srcdir)/doc/stage.ps $(top_distdir)/doc

docdir = $(prefix)/doc
install-data-hook:
	mkdir -p $(top_srcdir)/doc
	cp $(top_srcdir)/doc/stage.ps $(DESTDIR)$(prefix)/doc

uninstall-local:
	rm -f $(DESTDIR)$(docdir)/*

