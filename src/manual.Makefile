# system-wide compile flags - target-specific flags are added to each target below
CPPFLAGS = -Wall -I. -I$(top_srcdir)/replace  `pkg-config --cflags gtkglext-1.0`
LIBS = `pkg-config --libs gtkglext-1.0`

SOURCES =  \
	block.cc \
	gui.h \
	gui_gl.c \
	gui_gtk.cc \
	gui_gtk_prefs.c \
	matrix.c \
	model.cc \
	model.hh \
	model_callbacks.cc \
	model_laser.cc \
	model_load.cc \
	model_props.cc \
	raytrace.cc \
	stage.c \
	stage.h \
	stage_internal.h \
	stagecpp.cc \
	stest.cc \
	world.cc \
	worldfile.cc \
	worldfile.hh 

stest: $(SOURCES)
	g++ -I.. -I../replace  $(CPPFLAGS) $(SOURCES) $(LIBS) -o stest

