REFEREES = myref.so
PREFIX = /home/alem/school/playerstage

######## probably should leave this unchanged #########

STAGE_CFLAGS = `pkg-config stage --cflags`
STAGE_LIBS = `pkg-config stage --libs`

INCLUDE = -I. -I.. -I../.. -I../../replace

CFLAGS = -g -shared $(STAGE_CFLAGS) $(INCLUDE)
LIBS = $(STAGE_LIBS) -lstageplugin

INSTALL = install
LIBTOOL = libtool

all: $(REFEREES)

%.so: %.cc Makefile
	$(LIBTOOL) --mode=link g++ $(CFLAGS) -o $@ $< $(LIBS)

install:
	for i in $(REFEREES); \
	do $(LIBTOOL) --mode=install $(INSTALL) $$i $(PREFIX)/lib; \
	done

clean:
	rm -f myref.so

uninstall:
	rm -f $(PREFIX)/lib/myref.so
