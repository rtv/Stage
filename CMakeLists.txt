PROJECT(Stage)

SET (STAGE_VERSION 3.0.0 CACHE STRING "Stage distribution version")
SET (STAGE_API_VERSION 3.0.0 CACHE STRING "Stage API version")
MESSAGE (STATUS "${PROJECT_NAME} version ${STAGE_VERSION}")

include(FindPkgConfig)
pkg_check_modules( GLIB glib-2.0 )

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
