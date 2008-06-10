PROJECT(Stage)

SET( V_MAJOR 3 )
SET( V_MINOR 0 )
SET( V_BUGFIX 0 )

SET( VERSION ${V_MAJOR}.${V_MINOR}.${V_BUGFIX} )
SET( APIVERSION ${V_MAJOR}.${V_MINOR} )

cmake_minimum_required( VERSION 2.4 )

cmake_policy( SET CMP0003 NEW )
cmake_policy( SET CMP0005 NEW )

MESSAGE (STATUS "${PROJECT_NAME} version ${VERSION}")

# define some preprocessor strings that the code is expecting
ADD_DEFINITIONS( -DPROJECT=\"${PROJECT_NAME}\" 
		-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"
		-DVERSION=\"${VERSION}\" )

include(FindPkgConfig)
#variable_watch(PLAYER_FOUND)
#variable_watch(PLAYER_VERSION)
pkg_search_module( GLIB glib-2.0 )
pkg_search_module( PLAYER playercore )
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
			 ${CMAKE_INCLUDE_PATH}
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

string( REGEX MATCH "([0-9]*)\\.([0-9]*)\\." player_ver ${PLAYER_VERSION} )
SET(min_player_major 2)
SET(min_player_minor 1)
IF( NOT CMAKE_MATCH_1 LESS min_player_major )
	IF( NOT CMAKE_MATCH_2 LESS min_player_minor )
		MESSAGE( "Found Player v${PLAYER_VERSION} in ${PLAYER_INCLUDE_DIRS}")
		ADD_SUBDIRECTORY(libstageplugin)
	ENDIF()
ENDIF()

INSTALL(FILES rgb.txt stagelogo.png
        DESTINATION share/stage 
)

FILE( GLOB assets "assets/*.png" )
INSTALL(FILES ${assets}
        DESTINATION share/stage/assets
)


