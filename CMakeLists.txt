PROJECT(Stage)

SET( V_MAJOR 3 )
SET( V_MINOR 0 )
SET( V_BUGFIX 0 )

SET( VERSION ${V_MAJOR}.${V_MINOR}.${V_BUGFIX} )

MESSAGE (STATUS "${PROJECT_NAME} version ${VERSION}")

# define some preprocessor strings that the code is expecting
ADD_DEFINITIONS( -DPROJECT='"'${PROJECT_NAME}'"' 
		 -DVERSION='"'${VERSION}'"' )

include(FindPkgConfig)
pkg_check_modules( GLIB glib-2.0 )
pkg_check_modules( PLAYER playercore )
find_package(OPENGL REQUIRED)

# include(FindFLTK) doesn't seem to work for me, so we work a bit harder
set(FLTK_INCLUDE_DIR CMAKE_INCLUDE_PATH ) 
find_package(FLTK REQUIRED)

# all targets need these include directories
include_directories( . 
		     libstage 
		     replace 
		     ${GLIB_INCLUDE_DIRS} 
		     ${FLTK_INCLUDE_DIR}
		     ${OPENGL_INCLUDE_DIR}		     
)


# all targets need these library directories
link_directories(${GLIB_LIBRARY_DIRS}
		 ${FLTK_LIBRARY_DIRS}
		 ${OPENGL_LIBRARY_DIRS}
)

# generate a configuration header file in this dir
# CONFIGURE_FILE( config.h.cmake config.h )

# work through these subdirs
ADD_SUBDIRECTORY(libstage)
ADD_SUBDIRECTORY(examples)

IF( PLAYER_FOUND )
MESSAGE( "Found Player in ${PLAYER_INCLUDE_DIRS}")
ADD_SUBDIRECTORY(libstageplugin)
ENDIF( PLAYER_FOUND )

INSTALL(FILES rgb.txt stagelogo.png
        DESTINATION share 
)


