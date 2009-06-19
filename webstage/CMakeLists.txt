MESSAGE( STATUS "Configuring webstage" )

#include(FindPkgConfig)

#pkg_search_module( LIBXML2 REQUIRED libxml-2.0)
#IF( LIBXML2_FOUND )
#  MESSAGE( STATUS ${INDENT} "libxml version ${LIBXML2_VERSION} detected at ${LIBXML2_PREFIX}" )
#ELSE( LIBXML2_FOUND )
#  MESSAGE( ${INDENT} "libxml2 not detected" )
#ENDIF( LIBXML2_FOUND )

#include_directories( ${LIBXML2_INCLUDE_DIRS} )

add_executable( webstage webstage.cc )

target_link_libraries( webstage  ${WEBSIM_LIBRARIES} stage )

INSTALL(TARGETS webstage
	RUNTIME DESTINATION bin
)

