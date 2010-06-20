SET( expand_swarmSrcs expand_swarm.cc )
ADD_LIBRARY( expand_swarm MODULE ${expand_swarmSrcs} )
TARGET_LINK_LIBRARIES( expand_swarm stage )
set_source_files_properties( ${expand_swarmSrcs} PROPERTIES COMPILE_FLAGS "${FLTK_CFLAGS}" )
SET_TARGET_PROPERTIES( expand_swarm PROPERTIES PREFIX "" )

SET( expand_pioneerSrcs expand_pioneer.cc )
ADD_LIBRARY( expand_pioneer MODULE ${expand_pioneerSrcs} )
TARGET_LINK_LIBRARIES( expand_pioneer stage )
set_source_files_properties( ${expand_pioneerSrcs} PROPERTIES COMPILE_FLAGS "${FLTK_CFLAGS}" )
SET_TARGET_PROPERTIES( expand_pioneer PROPERTIES PREFIX "" )

INSTALL( TARGETS expand_swarm expand_pioneer DESTINATION ${PROJECT_PLUGIN_DIR})
