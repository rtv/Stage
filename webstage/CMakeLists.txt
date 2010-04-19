MESSAGE( STATUS "Looking for WebSim" )

pkg_search_module( WEBSIM websim )
IF( WEBSIM_FOUND )
  MESSAGE( STATUS ${INDENT} "WebSim version ${WEBSIM_VERSION} detected at ${WEBSIM_PREFIX}" )
  MESSAGE( STATUS "    WEBSIM_CFLAGS = ${WEBSIM_CFLAGS}" )
  MESSAGE( STATUS "    WEBSIM_LDFLAGS = ${WEBSIM_LDFLAGS}" )
  MESSAGE( STATUS "    WEBSIM_INCLUDE_DIRS = ${WEBSIM_INCLUDE_DIRS}" )
  MESSAGE( STATUS "    WEBSIM_LIBRARY_DIRS = ${WEBSIM_LIBRARY_DIRS}" )
  MESSAGE( STATUS "Configuring webstage" )
  include_directories( ${WEBSIM_INCLUDE_DIRS} )
  link_directories( ${WEBSIM_LIBRARY_DIRS} )
  add_executable( webstage webstage.cc )
  target_link_libraries( webstage stage event ${WEBSIM_LIBRARIES} )

  INSTALL(TARGETS webstage RUNTIME DESTINATION bin )

ELSE( WEBSIM_FOUND )
  MESSAGE( ${INDENT} "WebSim not detected. If WebSim is installed but not detected, check your PKG_CONFIG_PATH." )
ENDIF( WEBSIM_FOUND )




