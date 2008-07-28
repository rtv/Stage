PROJECT(Stage)

SET( V_MAJOR 3 )
SET( V_MINOR 0 )
SET( V_BUGFIX 0 )

SET( VERSION ${V_MAJOR}.${V_MINOR}.${V_BUGFIX} )
SET( APIVERSION ${V_MAJOR}.${V_MINOR} )

# minimum version of Player to build the plugin
SET( MIN_PLAYER 2.1.0 )

OPTION (BUILD_PLAYER_PLUGIN "Build Player plugin" ON)
OPTION (BUILD_LSPTEST "Build Player plugin tests" OFF)

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

# find the easy modules
include(FindPkgConfig)
pkg_search_module( GLIB REQUIRED glib-2.0 )
pkg_search_module( LIBPNG REQUIRED libpng )
find_package( OpenGL REQUIRED )

IF (NOT OPENGL_GLU_FOUND)
	message(FATAL_ERROR "GLU not found, aborting")
ENDIF (NOT OPENGL_GLU_FOUND)

# Look for player v${MIN_PLAYER} or higher and set flags
#  built-ins don't work properly, so use pkg-config directly
FIND_PROGRAM (PKGCONFIG NAMES pkg-config)
IF (PKGCONFIG)
	EXECUTE_PROCESS (COMMAND pkg-config --atleast-version=${MIN_PLAYER} playercore --silence-errors
		RESULT_VARIABLE PLAYER_NOT_FOUND)
	IF (PLAYER_NOT_FOUND EQUAL 0)
		# found the correct version
		pkg_search_module( PLAYER playercore )
		MESSAGE(STATUS "Player headers found in ${PLAYER_INCLUDE_DIRS}")
		MESSAGE(STATUS "Player libs found in ${PLAYER_LIBRARY_DIRS}")
		SET (PLAYER_FOUND TRUE)
	ELSE (PLAYER_NOT_FOUND EQUAL 0)
		MESSAGE(STATUS "Player >=v${MIN_PLAYER} not found, skipping Player plugin")
		SET (PLAYER_FOUND FALSE)
	ENDIF (PLAYER_NOT_FOUND EQUAL 0)
ELSE (PKGCONFIG)
	MESSAGE (STATUS "pkg-config not found, skipping Player plugin")
ENDIF (PKGCONFIG)

# find FLTK and set flags
FIND_PROGRAM (FLTKCONFIG NAMES fltk-config)
IF (FLTKCONFIG)
    EXECUTE_PROCESS (COMMAND fltk-config --cxxflags --use-gl --use-images
        OUTPUT_VARIABLE FLTK_CFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS (COMMAND fltk-config --ldflags --use-gl --use-images
        OUTPUT_VARIABLE FLTK_LDFLAGS
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
		     /opt/local/include/
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
