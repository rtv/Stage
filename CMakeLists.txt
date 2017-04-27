cmake_minimum_required( VERSION 2.8.7 FATAL_ERROR )
PROJECT(Stage)

SET( V_MAJOR 4 )
SET( V_MINOR 3 )
SET( V_BUGFIX 0 )

SET( VERSION ${V_MAJOR}.${V_MINOR}.${V_BUGFIX} )
SET( APIVERSION ${V_MAJOR}.${V_MINOR} )

# minimum version of Player to build the plugin
SET( MIN_PLAYER 2.1.0 )

OPTION (BUILD_PLAYER_PLUGIN "Build Player plugin" ON)
OPTION (BUILD_LSPTEST "Build Player plugin tests" OFF)
OPTION (CPACK_CFG "[release building] generate CPack configuration files" ON)

# todo - this doesn't work yet. Run Stage headless with -g.
# OPTION (BUILD_GUI "Build FLTK-based GUI. If OFF, build a gui-less Stage useful e.g. for headless compute clusters." ON ) 

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

if( PROJECT_OS_OSX )
  set(CMAKE_MACOSX_RPATH 1)
  # uncomment this to force 32bit on OS X, for example if you can't build FLTK 64bit
  #SET( FORCE_ARCH "-arch i386" )

  # suppress warnings about deprecated stuff. Forgive an old project!
  SET( WALL "-Wall -Wno-deprecated-declarations" )

endif( PROJECT_OS_OSX)

#####################################
# Build type cflags
SET (OPTIMIZE "-O2")

SET (CMAKE_CXX_FLAGS_RELEASE "${FORCE_ARCH} ${OPTIMIZE} -DNDEBUG ${WALL}" CACHE INTERNAL "CXX Flags for release" FORCE)
SET (CMAKE_CXX_FLAGS_DEBUG "-ggdb  ${FORCE_ARCH} ${WALL} -DDEBUG" CACHE INTERNAL "CXX Flags for debug" FORCE)
SET (CMAKE_CXX_FLAGS_PROFILE "-ggdb -pg  ${FORCE_ARCH} ${WALL}" CACHE INTERNAL "CXX Flags for profile" FORCE)

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


MESSAGE( STATUS "Checking for required libraries..." )

find_package( JPEG REQUIRED )

find_package( PNG REQUIRED )

# deal with new missing X11 on OS X 10.8 Mountain Lion
# todo: fix this properly
# SET( PNG_LIBRARIES /opt/X11/lib/libpng.dylib )
# SET( PNG_INCLUDE_DIR /opt/X11/include )

set (FLTK_SKIP_FLUID TRUE) 
find_package( FLTK REQUIRED )
find_package( OpenGL REQUIRED )

IF( NOT OPENGL_GLU_FOUND )
  MESSAGE( FATAL_ERROR "OpenGL GLU not found, aborting" )
ENDIF( NOT OPENGL_GLU_FOUND )

SET( INDENT "  * " )
# MESSAGE( STATUS ${INDENT} "JPEG_INCLUDE_DIR = ${JPEG_INCLUDE_DIR}" )
# MESSAGE( STATUS ${INDENT} "JPEG_LIBRARIES = ${JPEG_LIBRARIES}" ) 
# MESSAGE( STATUS ${INDENT} "PNG_INCLUDE_DIR = ${PNG_INCLUDE_DIR}" )
# MESSAGE( STATUS ${INDENT} "PNG_LIBRARIES = ${PNG_LIBRARIES}" ) 
# MESSAGE( STATUS ${INDENT} "FLTK_LIBRARIES=${FLTK_LIBRARIES}" )
# MESSAGE( STATUS ${INDENT} "FLTK_INCLUDE_DIR=${FLTK_INCLUDE_DIR}" )
# MESSAGE( STATUS ${INDENT} "OpenGL GLU found at ${OPENGL_INCLUDE_DIR}" )
# MESSAGE( STATUS "    OPENGL_INCLUDE_DIR = ${OPENGL_INCLUDE_DIR}")
# MESSAGE( STATUS "    OPENGL_glu_LIBRARY = ${OPENGL_glu_LIBRARY}")

MESSAGE( STATUS "Checking for optional libraries..." )

# Format the fltk libraries for inclusion in stage.pc
SET(PC_LIBRARIES ${FLTK_LIBRARIES} ${OPENGL_LIBRARIES})
SET(PC_INCLUDE_DIRS ${FLTK_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})

SET(PC_LINK_FLAGS "")
FOREACH(LIB ${PC_LIBRARIES})
  GET_FILENAME_COMPONENT(LIBNAME ${LIB} NAME_WE)
  STRING(REGEX REPLACE "^lib" "" LINKLIB ${LIBNAME})
  SET(PC_LINK_FLAGS "${PC_LINK_FLAGS} -l${LINKLIB}")
ENDFOREACH(LIB ${FLTK_LIBRARIES})

SET(PC_INCLUDE_FLAGS "")
FOREACH(INC ${PC_INCLUDE_DIRS})
  SET(PC_INCLUDE_FLAGS "${PC_INCLUDE_FLAGS} -I${INC}")
ENDFOREACH(INC ${PC_INCLUDE_DIRS})

# Create the pkgconfig file
CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/stage.pc.in ${CMAKE_CURRENT_BINARY_DIR}/stage.pc @ONLY)
INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/stage.pc DESTINATION ${PROJECT_LIB_DIR}/pkgconfig/)

MESSAGE( STATUS "Installation path CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" )

# all targets need these include directories
include_directories( . 
		     libstage 
		     replace 
		     ${FLTK_INCLUDE_DIR}
                     ${PNG_INCLUDE_DIR}
                     ${JPEG_INCLUDE_DIR}
		     ${CMAKE_INCLUDE_PATH}
)


# work through these subdirs
ADD_SUBDIRECTORY(libstage)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(assets)
ADD_SUBDIRECTORY(worlds)
#ADD_SUBDIRECTORY(avonstage)		 

IF ( BUILD_PLAYER_PLUGIN )
  # Player does not have a CMake package, but does provide pkgconfig info
  include(FindPkgConfig)
  pkg_search_module( PLAYER playercore>=2.1.0 ) 
  IF( PLAYER_FOUND )
    MESSAGE( STATUS ${INDENT} "Player version ${PLAYER_VERSION} detected at ${PLAYER_PREFIX}" )
    MESSAGE( STATUS "    PLAYER_CFLAGS: ${PLAYER_CFLAGS}" )
    MESSAGE( STATUS "    PLAYER_LDFLAGS: ${PLAYER_LDFLAGS}"  )
  ELSE( PLAYER_FOUND )
    MESSAGE(STATUS ${INDENT} "Player not detected. If Player is installed but not detected, check your PKG_CONFIG_PATH." )
  ENDIF( PLAYER_FOUND )

  IF ( PLAYER_FOUND )
    ADD_SUBDIRECTORY(libstageplugin)
  ENDIF ( PLAYER_FOUND )
ENDIF ( BUILD_PLAYER_PLUGIN )

# Create the CMake module files (needs to be run after the stage target was created)
if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} GREATER 2.8.11)
  # Use a generator expression if the version of cmake allows it.
  set(STAGE_TARGET_NAME "$<TARGET_FILE_NAME:stage>")
else()
  # Otherwise use the LOCATION property of the target (this will produce a warning on newer versions of cmake)
  get_property(location_ TARGET stage PROPERTY LOCATION)
  get_filename_component(STAGE_TARGET_NAME "${location_}" NAME)
  unset(location_)
endif()
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/stage-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/stage-config.cmake @ONLY)
if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} GREATER 2.8.11)
  # Also run it through file(GENERATE ...) to expand generator expressions (if the version of cmake supports it).
  FILE (GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stage-config.cmake
    INPUT ${CMAKE_CURRENT_BINARY_DIR}/stage-config.cmake)
endif()
CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/stage-config-version.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/stage-config-version.cmake @ONLY)
INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/stage-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/stage-config-version.cmake DESTINATION ${PROJECT_LIB_DIR}/cmake/${PROJECT_NAME})

# generate a cpack config file used to create packaged tarballs
IF ( CPACK_CFG )
  INCLUDE(InstallRequiredSystemLibraries)
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME}: A Multiple Robot Simulator")
  SET(CPACK_PACKAGE_VENDOR "The Player Project")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
  SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")
  SET(CPACK_PACKAGE_VERSION_MAJOR "${V_MAJOR}")
  SET(CPACK_PACKAGE_VERSION_MINOR "${V_MINOR}")
  SET(CPACK_PACKAGE_VERSION_PATCH "${V_BUGFIX}")
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}")
  INCLUDE(CPack)
ENDIF ( CPACK_CFG )
