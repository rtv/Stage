SUBDIRS = .

if WITH_GNOME2
GNOMESTAGELIB = libgnomestage.a
else
GNOMESTAGELIB =
endif

AM_CPPFLAGS = -I$(top_srcdir)/src -I@PLAYER_DIR@/include -I@PLAYER_DIR@/server -I@PLAYER_DIR@/server/drivers/stage `pkg-config --cflags libgnomecanvas-2.0 libgnomeui-2.0`


noinst_LIBRARIES = $(GNOMESTAGELIB)

libgnomestage_a_SOURCES = gnomegui.cc gnomedevices.cc

