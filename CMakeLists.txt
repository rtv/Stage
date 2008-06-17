PROJECT(Stage)

SET( V_MAJOR 3 )
SET( V_MINOR 0 )
SET( V_BUGFIX 0 )

SET( VERSION ${V_MAJOR}.${V_MINOR}.${V_BUGFIX} )
SET( APIVERSION ${V_MAJOR}.${V_MINOR} )

# minimum version of Player to build the plugin
SET( MIN_PLAYER 2.2.0 ) # change once plugin is fixed

cmake_minimum_required( VERSION 2.4 FATAL_ERROR )

IF (CMAKE_MAJOR_VERSION EQUAL 2 AND NOT CMAKE_MINOR_VERSION LESS 6)
	cmake_policy( SET CMP0003 NEW )
	cmake_policy( SET CMP0005 OLD )
ENDIF (CMAKE_MAJOR_VERSION EQUAL 2 AND NOT CMAKE_MINOR_VERSION LESS 6)

MESSAGE (STATUS "${PROJECT_NAME} version ${VERSION}")

# define some preprocessor strings that the code is expecting
ADD_DEFINITIONS( -DPROJECT=\\\"${PROJECT_NAME}\\\" 
		-DINSTALL_PREFIX=\\\"${CMAKE_INSTALL_PREFIX}\\\"
		-DVERSION=\\\"${VERSION}\\\" )


# find the easy modules
include(FindPkgConfig)
pkg_search_module( GLIB REQUIRED glib-2.0 )
find_package( OpenGL REQUIRED )

pkg_search_module( PLAYER playercore )


# find FLTK and set flags
FIND_PROGRAM (FLTKCONFIG NAMES fltk-config)
IF (FLTKCONFIG)
    EXECUTE_PROCESS (COMMAND fltk-config --cxxflags --use-gl --use-images
        OUTPUT_VARIABLE FLTK_CFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS (COMMAND fltk-config --ldflags --use-gl --use-images
        OUTPUT_VARIABLE FLTK_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    MESSAGE (STATUS "Found FLTK")
    SET (FLTK_FOUND TRUE)
ELSE (FLTKCONFIG)
    MESSAGE (FATAL_ERROR "FLTK not found, aborting")
    SET (FLTK_FOUND FALSE)
ENDIF (FLTKCONFIG)

IF (APPLE)
	# define flags to avoid the error:
	#  ld: cycle in dylib re-exports with /usr/X11R6/lib/libGL.dylib
	MESSAGE(STATUS "Setting Apple specific flags")
	# work around the OS X 10.5  X11/OpenGL library bug
	SET(APPLE_LIBRARIES "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
ENDIF (APPLE)

# all targets need these include directories
include_directories( . 
		     libstage 
		     replace 
		     ${GLIB_INCLUDE_DIRS} 
		     ${FLTK_INCLUDE_DIR}
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

if( PLAYER_FOUND )
ADD_SUBDIRECTORY(libstageplugin)
endif( PLAYER_FOUND )	 


INSTALL(FILES rgb.txt stagelogo.png
        DESTINATION share/stage 
)

FILE( GLOB assets "assets/*.png" )
INSTALL(FILES ${assets}
        DESTINATION share/stage/assets
)


