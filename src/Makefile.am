
# Automake script to build Stage's Makefile
#
# RTV 20021026
# $Id: Makefile.am,v 1.30 2003-05-24 21:08:34 gerkey Exp $


SUBDIRS = models rtkstage 
#gnome2

stage_DEPENDENCIES = models/libstgmodels.a $(top_srcdir)/replace/libreplace.a  @GUI_DIR@/@GUI_LIB@ 

AM_CPPFLAGS = 	-Wall @PLAYER_CPPFLAGS@ -I$(top_builddir)/replace $(GUI_CPPFLAGS)

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

stage_LDADD = 	-lm -Lmodels -lstgmodels @PLAYER_LDADD@ @GUI_DIR@/@GUI_LIB@ @GUI_LDADD@ -L$(top_builddir)/replace -lreplace

