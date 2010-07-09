PROJECT(Stage)

SET( V_MAJOR 3 )
SET( V_MINOR 2 )
SET( V_BUGFIX 2 )

SET( VERSION ${V_MAJOR}.${V_MINOR}.${V_BUGFIX} )
SET( APIVERSION ${V_MAJOR}.${V_MINOR} )

# minimum version of Player to build the plugin
SET( MIN_PLAYER 2.1.0 )

OPTION (BUILD_PLAYER_PLUGIN "Build Player plugin" ON)
OPTION (BUILD_LSPTEST "Build Player plugin tests" OFF)
OPTION (CPACK_CFG "[release building] generate CPack configuration files" ON)

# todo - this doesn't work yet. Run Stage headless with -g.
# OPTION (BUILD_GUI "Build FLTK-based GUI. If OFF, build a gui-less Stage useful e.g. for headless compute clusters." ON ) 

cmake_minimum_required( VERSION 2.4 FATAL_ERROR )

IF (CMAKE_MAJOR_VERSION EQUAL 2 AND NOT CMAKE_MINOR_VERSION LESS 6)
	cmake_policy( SET CMP0003 NEW )
	cmake_policy( SET CMP0005 OLD )
ENDIF (CMAKE_MAJOR_VERSION EQUAL 2 AND NOT CMAKE_MINOR_VERSION LESS 6)

MESSAGE (STATUS "${PROJECT_NAME} version ${VERSION}")

# Set where to find our internal CMake scripts
SET (PROJECT_CMAKE_DIR ${PROJECT_SOURCE_DIR}/cmake CACHE PATH "Location of CMake scripts")
# Determine the operating system in detail
INCLUDE (${PROJECT_CMAKE_DIR}/internal/FindOS.cmake)
INCLUDE (${PROJECT_CMAKE_DIR}/internal/UninstallTarget.cmake)
# Enable -Wall by default unless on Win or Solaris
IF (NOT PROJECT_OS_WIN AND NOT PROJECT_OS_SOLARIS)
    # Using -Wall on Windows causes MSVC to produce thousands of warnings in its
    # own standard headers, dramatically slowing down the build.
    SET (WALL "-Wall " )
ENDIF (NOT PROJECT_OS_WIN AND NOT PROJECT_OS_SOLARIS)

#####################################
# Build type cflags
SET (CMAKE_CXX_FLAGS_RELEASE "  -DNDEBUG ${WALL} " CACHE INTERNAL "C Flags for release" FORCE)
SET (CMAKE_CXX_FLAGS_DEBUG " -ggdb  ${WALL} " CACHE INTERNAL "C Flags for debug" FORCE)
SET (CMAKE_CXX_FLAGS_PROFILE "  -ggdb -pg ${WALL} " CACHE INTERNAL "C Flags for profile" FORCE)

#####################################
# Set the default build type
IF (NOT CMAKE_BUILD_TYPE)
  SET (CMAKE_BUILD_TYPE "release" CACHE STRING 
    "Choose the type of build, options are: release (default) debug profile" FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)
STRING(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

MESSAGE( STATUS "Build type ${CMAKE_BUILD_TYPE}" )

ENABLE_TESTING()

SET(RGBFILE ${CMAKE_INSTALL_PREFIX}/share/stage/rgb.txt )

# Create the config.h file
# config.h belongs with the source (and not in CMAKE_CURRENT_BINARY_DIR as in Brian's original version)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in 
					${CMAKE_CURRENT_SOURCE_DIR}/config.h @ONLY)

message( STATUS "Checking for libtool" )
find_path( LTDL_INCLUDE_DIR ltdl.h DOC "Libtool include dir" )
find_library( LTDL_LIB ltdl DOC "Libtool lib" )

include_directories( 
	${OPENGL_INCLUDE_DIR}
	${LTDL_INCLUDE_DIR}

)

# find the easy modules
include(FindPkgConfig)

MESSAGE( STATUS "Checking for required libraries..." )

SET( INDENT "  * " )

pkg_search_module( LIBPNG REQUIRED libpng )
IF( LIBPNG_FOUND )
#  MESSAGE( STATUS ${INDENT} "Libpng version ${LIBPNG_VERSION} detected at ${LIBPNG_PREFIX}" )
#  MESSAGE( STATUS "    LIBPNG_CFLAGS = ${LIBPNG_CFLAGS}" )
#  MESSAGE( STATUS "    LIBPNG_LDFLAGS = ${LIBPNG_LDFLAGS}" )
ELSE( LIBPNG_FOUND )
  MESSAGE( ${INDENT} "Libpng not detected" )
ENDIF( LIBPNG_FOUND )

#MESSAGE( STATUS "BUILD_GUI is ${BUILD_GUI}" )
#IF( BUILD_GUI )

## the FLTK package script is not useful - it finds an X11-based FLTK on OS X.
## so we can't do this stuff
#find_package( FLTK REQUIRED )
#IF( FLTK_FOUND )
#  MESSAGE( STATUS ${INDENT} "FLTK_LIBRARIES=${FLTK_LIBRARIES}" )
#  MESSAGE( STATUS ${INDENT} "FLTK_INCLUDE_DIR=${FLTK_INCLUDE_DIR}" )
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
		 

OPTION (LINK_FLTK_STATICALLY "Link to the FLTK libraries statically" OFF)
IF (LINK_FLTK_STATICALLY)
	SET (FLTK_CONFIG_CMD "--ldstaticflags")
	MESSAGE (STATUS "Linking to FLTK statically.")
ELSE (LINK_FLTK_STATICALLY)
	SET (FLTK_CONFIG_CMD "--ldflags")
	#MESSAGE (STATUS "Linking to FLTK dynamically.")
ENDIF (LINK_FLTK_STATICALLY)

EXECUTE_PROCESS ( COMMAND ${FLTK_CONFIG} --version
  OUTPUT_VARIABLE FLTK_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE )
MESSAGE( STATUS ${INDENT} "FLTK version ${FLTK_VERSION} detected via ${FLTK_CONFIG}")
EXECUTE_PROCESS ( COMMAND ${FLTK_CONFIG} --use-gl --use-images --cxxflags 
  OUTPUT_VARIABLE FLTK_CFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE )
EXECUTE_PROCESS ( COMMAND ${FLTK_CONFIG} --use-gl --use-images ${FLTK_CONFIG_CMD}
  OUTPUT_VARIABLE FLTK_LDFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE )
EXECUTE_PROCESS ( COMMAND ${FLTK_CONFIG} --use-gl --use-images --libs
  OUTPUT_VARIABLE FLTK_LIBS_NEWLINES
  OUTPUT_STRIP_TRAILING_WHITESPACE )
# replace the newlines with semicolons
STRING(REGEX REPLACE "\n" ";" FLTK_LIBS "${FLTK_LIBS_NEWLINES}")

#MESSAGE( STATUS "    FLTK_CFLAGS = ${FLTK_CFLAGS}")
#MESSAGE( STATUS "    FLTK_LDFLAGS = ${FLTK_LDFLAGS}")
#MESSAGE( STATUS "    FLTK_LIBS = ${FLTK_LIBS}")

STRING(REGEX REPLACE " -lGLU" "" TMP "${FLTK_LDFLAGS}")
STRING(REGEX REPLACE " -lGL" "" TMP2 "${TMP}")
STRING(REGEX REPLACE " -lpng" "" FLTK_LDFLAGS_PKGCONFIG "${TMP2}")


STRING(REGEX REPLACE " -lgl" "" FLTK_LIBS_PKGCONFIG "${FLTK_LIBS}")
STRING(REGEX REPLACE " -lglu" "" FLTK_LIBS_PKGCONFIG" "${FLTK_LIBS_PKGCONFIG}")
STRING(REGEX REPLACE " -lpng" "" FLTK_LIBS_PKGCONFIG" "${FLTK_LIBS_PKGCONFIG}")


SET (FLTK_FOUND TRUE)

# ENDIF( BUILD_GUI )

#MESSAGE( ${INDENT} "Checking for OpenGL" )
find_package( OpenGL REQUIRED )
IF( NOT OPENGL_GLU_FOUND )
  MESSAGE( FATAL_ERROR "OpenGL GLU not found, aborting" )
ENDIF( NOT OPENGL_GLU_FOUND )
#MESSAGE( STATUS ${INDENT} "OpenGL GLU found at ${OPENGL_INCLUDE_DIR}" )
#MESSAGE( STATUS "    OPENGL_INCLUDE_DIR = ${OPENGL_INCLUDE_DIR}")
#MESSAGE( STATUS "    OPENGL_glu_LIBRARY = ${OPENGL_glu_LIBRARY}")


MESSAGE( STATUS "Checking for optional libraries..." )

pkg_search_module( PLAYER playercore>=2.1.0 ) 
IF( PLAYER_FOUND )
  MESSAGE( STATUS ${INDENT} "Player version ${PLAYER_VERSION} detected at ${PLAYER_PREFIX}" )
#  MESSAGE( STATUS "    PLAYER_CFLAGS: ${PLAYER_CFLAGS}" )
#  MESSAGE( STATUS "    PLAYER_LDFLAGS: ${PLAYER_LDFLAGS}"  )
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

# Create the pkgconfig file
CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/stage.pc.in ${CMAKE_CURRENT_BINARY_DIR}/stage.pc @ONLY)
INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/stage.pc DESTINATION ${PROJECT_LIB_DIR}/pkgconfig/)

MESSAGE( STATUS "Installation path CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" )

# all targets need these include directories
include_directories( . 
		     libstage 
		     replace 
		     ${LIBPNG_INCLUDE_DIRS}
		     ${CMAKE_INCLUDE_PATH}
)


# all targets need these library directories
link_directories( 
	${LIBPNG_LIBRARY_DIRS}
)

# work through these subdirs
ADD_SUBDIRECTORY(libstage)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(assets)
ADD_SUBDIRECTORY(worlds)
ADD_SUBDIRECTORY(webstage)		 

IF ( BUILD_PLAYER_PLUGIN AND PLAYER_FOUND )
  ADD_SUBDIRECTORY(libstageplugin)
ENDIF ( BUILD_PLAYER_PLUGIN AND PLAYER_FOUND )	 


# generate a cpack config file used to create packaged tarballs
IF ( CPACK_CFG )
  INCLUDE(InstallRequiredSystemLibraries)
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME}: A Multiple Robot Simulator")
  SET(CPACK_PACKAGE_VENDOR "The Player Project")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
  SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")
  SET(CPACK_PACKAGE_VERSION_MAJOR "${V_MAJOR}")
  SET(CPACK_PACKAGE_VERSION_MINOR "${V_MINOR}")
  SET(CPACK_PACKAGE_VERSION_PATCH "${V_BUGFIX}")
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}")
  INCLUDE(CPack)
ENDIF ( CPACK_CFG )
