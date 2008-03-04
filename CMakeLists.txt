PROJECT(Stage)

include(FindPkgConfig)
pkg_check_modules( GLIB gdk-pixbuf-2.0 )

find_package(OPENGL REQUIRED)

if(APPLE)
  set(FLTK_INCLUDE_DIR /opt/local/include) # MacPorts' install path
endif(APPLE)
find_package(FLTK REQUIRED)

include_directories( .
		     replace 
		     libstage
		     ${GLIB_INCLUDE_DIRS} 
		     ${FLTK_INCLUDE_DIR}
		     ${OPENGL_INCLUDE_DIR}
)

ADD_SUBDIRECTORY(libstage)
ADD_SUBDIRECTORY(examples)
