
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.25.2.4.2.1 2004-11-12 23:47:52 gerkey Exp $


SUBDIRS = models rtkstage

stage_DEPENDENCIES = models/libstgmodels.a @GUI_LIBPATH@

AM_CPPFLAGS = 	-Wall @PLAYER_CFLAGS@ $(GUI_CPPFLAGS)

bin_PROGRAMS = stage


stage_SOURCES = \
         client.cc \
         colors.cc \
         colors.hh \
         entity.cc \
         entity.hh \
         gui.hh \
         library.cc \
         library.hh \
         main.cc \
         matrix.cc \
         matrix.hh \
         nogui.cc \
         playerdevice.cc \
         playerdevice.hh \
         raytrace.cc \
         raytrace.hh \
         replace.h \
         server.cc \
         server.hh \
         stageio.cc \
         stage_types.hh \
         world.cc \
         worldfile.cc \
         worldfile.hh \
         world.hh

stage_LDADD = 	-lm -Lmodels -lstgmodels @PLAYER_LIBS@ @GUI_LIBPATH@ @GUI_LDADD@

