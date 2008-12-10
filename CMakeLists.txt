PROJECT(Stage)

SET( V_MAJOR 3 )
SET( V_MINOR 1 )
SET( V_BUGFIX 0 )

SET( VERSION ${V_MAJOR}.${V_MINOR}.${V_BUGFIX} )
SET( APIVERSION ${V_MAJOR}.${V_MINOR} )

# minimum version of Player to build the plugin
SET( MIN_PLAYER 2.1.0 )

OPTION (BUILD_PLAYER_PLUGIN "Build Player plugin" ON)
OPTION (BUILD_LSPTEST "Build Player plugin tests" OFF)
OPTION (CPACK_CFG "[release building] generate CPack configuration files" OFF)

cmake_minimum_required( VERSION 2.4 FATAL_ERROR )

IF (CMAKE_MAJOR_VERSION EQUAL 2 AND NOT CMAKE_MINOR_VERSION LESS 6)
	cmake_policy( SET CMP0003 NEW )
	cmake_policy( SET CMP0005 OLD )
ENDIF (CMAKE_MAJOR_VERSION EQUAL 2 AND NOT CMAKE_MINOR_VERSION LESS 6)

MESSAGE (STATUS "${PROJECT_NAME} version ${VERSION}")

ENABLE_TESTING()

# Create the pkgconfig file
CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/stage.pc.in ${CMAKE_CURRENT_BINARY_DIR}/stage.pc @ONLY)
INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/stage.pc DESTINATION lib/pkgconfig/)

# Create the config.h file
SET(RGBFILE ${CMAKE_INSTALL_PREFIX}/share/stage/rgb.txt )

# config.h belongs with the source (and not in CMAKE_CURRENT_BINARY_DIR as in Brian's original version)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in 
					${CMAKE_CURRENT_SOURCE_DIR}/config.h @ONLY)

message( STATUS "Checking for libtool" )
find_path( LTDL_INCLUDE_DIR ltdl.h DOC "Libtool include dir" )
find_library( LTDL_LIB ltdl DOC "Libtool lib" )

IF (${LTDL_LIB} STREQUAL "LTDL_LIB-NOTFOUND")
	message( FATAL_ERROR "libtool library not found, aborting" )
ELSE (${LTDL_LIB} STREQUAL "LTDL_LIB-NOTFOUND")
	message( STATUS "  found" )
ENDIF (${LTDL_LIB} STREQUAL "LTDL_LIB-NOTFOUND")

include_directories( 
	${OPENGL_INCLUDE_DIR}
	${LTDL_INCLUDE_DIR}
)

# find the easy modules
include(FindPkgConfig)

MESSAGE( STATUS "Checking for required libraries..." )

SET( INDENT "  * " )
pkg_search_module( GLIB REQUIRED gthread-2.0 )
IF( GLIB_FOUND )
  MESSAGE( ${INDENT} "Glib version ${GLIB_VERSION} detected at ${GLIB_PREFIX}" )
  MESSAGE( "    GLIB_CFLAGS = ${GLIB_CFLAGS}" )
  MESSAGE( "    GLIB_LDFLAGS = ${GLIB_LDFLAGS}" )
ELSE( GLIB_FOUND )
  MESSAGE( ${INDENT} "Glib not detected" )
ENDIF( GLIB_FOUND )

pkg_search_module( LIBPNG REQUIRED libpng )
IF( LIBPNG_FOUND )
  MESSAGE( ${INDENT} "Libpng version ${LIBPNG_VERSION} detected at ${LIBPNG_PREFIX}" )
  MESSAGE( "    LIBPNG_CFLAGS = ${LIBPNG_CFLAGS}" )
  MESSAGE( "    LIBPNG_LDFLAGS = ${LIBPNG_LDFLAGS}" )
ELSE( LIBPNG_FOUND )
  MESSAGE( ${INDENT} "Libpng not detected" )
ENDIF( LIBPNG_FOUND )

## the FLTK package script is not useful - it finds an X11-based FLTK on OS X.
## so we can't do this stuff
#find_package( FLTK REQUIRED )
#IF( FLTK_FOUND )
#  MESSAGE( ${INDENT} "FLTK_LIBRARIES=${FLTK_LIBRARIES}" )
#  MESSAGE( ${INDENT} "FLTK_INCLUDE_DIR=${FLTK_INCLUDE_DIR}" )
#ELSE( FLTK_FOUND )
#  MESSAGE( FATAL_ERROR "FLTK not found, aborting" )
#ENDIF( FLTK_FOUND )

## and we have to do all this instead
IF ( NOT FLTK_CONFIG )
   FIND_PROGRAM ( FLTK_CONFIG NAMES fltk-config PATHS ${FLTK_CONFIG_PATH} )
ENDIF ( NOT FLTK_CONFIG )
IF ( NOT FLTK_CONFIG )
  MESSAGE( FATAL_ERROR "Cannot find fltk-config, aborting" )
ENDIF( NOT FLTK_CONFIG)
		 
EXECUTE_PROCESS ( COMMAND ${FLTK_CONFIG} --version
  OUTPUT_VARIABLE FLTK_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE )
MESSAGE( ${INDENT} "FLTK version ${FLTK_VERSION} detected via ${FLTK_CONFIG}")
EXECUTE_PROCESS ( COMMAND ${FLTK_CONFIG} --use-gl --use-images --cxxflags 
  OUTPUT_VARIABLE FLTK_CFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE )
EXECUTE_PROCESS ( COMMAND ${FLTK_CONFIG} --use-gl --use-images --ldstaticflags
  OUTPUT_VARIABLE FLTK_LDFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE )
EXECUTE_PROCESS ( COMMAND ${FLTK_CONFIG} --use-gl --use-images --libs
  OUTPUT_VARIABLE FLTK_LIBS_NEWLINES
  OUTPUT_STRIP_TRAILING_WHITESPACE )
# replace the newlines with semicolons
STRING(REGEX REPLACE "\n" ";" FLTK_LIBS "${FLTK_LIBS_NEWLINES}")

MESSAGE( "    FLTK_CFLAGS = ${FLTK_CFLAGS}")
MESSAGE( "    FLTK_LDFLAGS = ${FLTK_LDFLAGS}")
MESSAGE( "    FLTK_LIBS = ${FLTK_LIBS}")

SET (FLTK_FOUND TRUE)

#MESSAGE( ${INDENT} "Checking for OpenGL" )
find_package( OpenGL REQUIRED )
IF( NOT OPENGL_GLU_FOUND )
  MESSAGE( FATAL_ERROR "OpenGL GLU not found, aborting" )
ENDIF( NOT OPENGL_GLU_FOUND )
MESSAGE( ${INDENT} "OpenGL GLU found at ${OPENGL_INCLUDE_DIR}" )
MESSAGE( "    OPENGL_INCLUDE_DIR = ${OPENGL_INCLUDE_DIR}")
MESSAGE( "    OPENGL_glu_LIBRARY = ${OPENGL_glu_LIBRARY}")


MESSAGE( STATUS "Checking for optional libraries..." )

pkg_search_module( PLAYER playercore>=2.1.0 ) 
IF( PLAYER_FOUND )
  MESSAGE( ${INDENT} "Player version ${PLAYER_VERSION} detected at ${PLAYER_PREFIX}" )
  MESSAGE( "    PLAYER_CFLAGS: ${PLAYER_CFLAGS}" )
  MESSAGE( "    PLAYER_LDFLAGS: ${PLAYER_LDFLAGS}" )
ELSE( PLAYER_FOUND )
  MESSAGE( ${INDENT} "Player not detected. If Player is installed but not detected, check your PKG_CONFIG_PATH." )
ENDIF( PLAYER_FOUND )


## this should not be necessary now that we do the FLTK config
## carefully above. I'll leave the code here in case of trouble
# IF (APPLE)
#	# define flags to avoid the error:
#	#  ld: cycle in dylib re-exports with /usr/X11R6/lib/libGL.dylib
#	MESSAGE(STATUS "Setting Apple specific flags")
#	# work around the OS X 10.5  X11/OpenGL library bug
#	SET(APPLE_LIBRARIES "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
#ENDIF (APPLE)

# all targets need these include directories
include_directories( . 
		     libstage 
		     replace 
		     ${GLIB_INCLUDE_DIRS}
		     ${LIBPNG_INCLUDE_DIRS}
		     ${CMAKE_INCLUDE_PATH}
)


# all targets need these library directories
link_directories(${GLIB_LIBRARY_DIRS}
	${GLIB_LIBRARY_DIRS}
	${LIBPNG_LIBRARY_DIRS}
)

# work through these subdirs
ADD_SUBDIRECTORY(libstage)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(assets)
ADD_SUBDIRECTORY(worlds)

IF ( BUILD_PLAYER_PLUGIN AND PLAYER_FOUND )
  ADD_SUBDIRECTORY(libstageplugin)
ENDIF ( BUILD_PLAYER_PLUGIN AND PLAYER_FOUND )	 


# generate a cpack config file used to create packaged tarballs
IF ( CPACK_CFG )
  INCLUDE(InstallRequiredSystemLibraries)
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME}: A Multiple Robot Simulator")
  SET(CPACK_PACKAGE_VENDOR "The Player Project")
  #SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/DESCRIPTION.txt")
  #SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
  #SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")
  SET(CPACK_PACKAGE_VERSION_MAJOR "${V_MAJOR}")
  SET(CPACK_PACKAGE_VERSION_MINOR "${V_MINOR}")
  SET(CPACK_PACKAGE_VERSION_PATCH "${V_BUGFIX}")
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}")
  INCLUDE(CPack)
ENDIF ( CPACK_CFG )
