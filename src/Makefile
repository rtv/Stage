
##############################################################################
# include the main configurations from the top-level directory
#

include ../Makefile.common

##############################################################################
# Build section

# choose one of these that suits your build (debug/profile/production)

# production
#CCFLAGS = -O2 
# debug
CCFLAGS = -g3 
# debug + profile
#CCFLAGS = -pg -g3 

# Flags common to all targets
#
INCLUDES = -I../include -I$(PLAYER_INC) -I$(PLAYER_DIR)/client_libs/c \
           -I$(PLAYER_DIR)/client_libs/c++
CCFLAGS += -Wall -DREENTRANT $(ZLIB_FLAGS) $(INCLUDES)
CCFLAGS += -DVERSION=\"$(VERSION)\"

# production/debug link
LINKFLAGS = -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
# profile link
#LINKFLAGS = -pg -lm -lstdc++ -lpthread  $(ZLIB_LIBS)
LINKFLAGS += $(OLDLIBS)


##############################################################################
# Targets

all: stage rtkstage xs truthlog 

stage:
	cd stage_objs && make stage

rtkstage:
	cd rtkstage_objs && make rtkstage

# Target XS - the X Player/Stage Interface
#

xs :CCFLAGS += -I/usr/X11R6/include 
xs: LINKFLAGS += -L/usr/X11R6/lib -lX11 -L$(PLAYER_LIB) \
                  -L$(PLAYER_DIR)/client_libs/c++ -lplayerclient
xs : xs.cc render.cc
	$(CC) xs.cc render.cc $(CCFLAGS) $(LINKFLAGS) -o xs

truthlog: truthlog.cc 
	$(CC) -I../include -I$(PLAYER_INC) -lstdc++ truthlog.cc -o truthlog

manager: manager.cc 
	$(CC) -I../include -Wall -I$(PLAYER_INC) -lstdc++ manager.cc -o manager

clean:
	rm -f *.o *~ core xs truthlog 
	cd stage_objs && make clean
	cd rtkstage_objs && make clean

dep:
	$(MAKEDEP) -Y -I../include -I$(PLAYER_INC) -s "# Dependancies (make dep)" truthlog.cc xs.cc render.cc 2>/dev/null
	$(RM) -f Makefile.bak
	cd stage_objs && make dep
	cd rtkstage_objs && make dep

clean_dep:
	$(MAKEDEP) -s "# Dependancies (make dep)" 
	$(RM) -f Makefile.bak
	cd stage_objs && make clean_dep
	cd rtkstage_objs && make clean_dep

%.o : %.cc
	${CC} ${CCFLAGS} -c $< -o $@

# Dependancies (make dep)

truthlog.o: ../include/truthserver.hh ../include/stage_types.hh
xs.o: ../include/stage_types.hh ../include/truthserver.hh ../include/xs.hh
render.o: ../include/stage_types.hh ../include/truthserver.hh
render.o: ../include/xs.hh
