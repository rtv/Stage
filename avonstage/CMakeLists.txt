MESSAGE( STATUS "Looking for Avon" )

pkg_search_module( AVON avon )
IF( AVON_FOUND )
  MESSAGE( STATUS ${INDENT} "Avon version ${AVON_VERSION} detected at ${AVON_PREFIX}" )
  MESSAGE( STATUS "    AVON_CFLAGS = ${AVON_CFLAGS}" )
  MESSAGE( STATUS "    AVON_LDFLAGS = ${AVON_LDFLAGS}" )
  MESSAGE( STATUS "    AVON_INCLUDE_DIRS = ${AVON_INCLUDE_DIRS}" )
  MESSAGE( STATUS "    AVON_LIBRARY_DIRS = ${AVON_LIBRARY_DIRS}" )
  MESSAGE( STATUS "Configuring avonstage" )
  include_directories( ${AVON_INCLUDE_DIRS} )
  link_directories( ${AVON_LIBRARY_DIRS} )
  add_executable( avonstage avonstage.cc )
  target_link_libraries( avonstage stage ${AVON_LIBRARIES} )

	set_target_properties( avonstage PROPERTIES LINK_FLAGS "${FLTK_LDFLAGS}" )

  INSTALL(TARGETS avonstage RUNTIME DESTINATION bin )

ELSE( AVON_FOUND )
  MESSAGE( STATUS ${INDENT} "Avon not detected. If Avon is installed but not detected, check your PKG_CONFIG_PATH." )
ENDIF( AVON_FOUND )




