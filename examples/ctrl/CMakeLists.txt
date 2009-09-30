
SET( PLUGINS
     fasr
     lasernoise
     sink
     source
     wander
	  wander_pioneer
	  pioneer_flocking
     rasterize
	  dynamic
)

# need plaer's wavefront planning library for this one
#if( PLAYER_FOUND )  
#  SET( PLUGINS ${PLUGINS} fasr_plan )
#endif( PLAYER_FOUND )

set_source_files_properties( ${PLUGINS} PROPERTIES COMPILE_FLAGS "${FLTK_CFLAGS}" )

# create a library module for each plugin and link libstage to each

foreach( PLUGIN ${PLUGINS} )
  ADD_LIBRARY( ${PLUGIN} MODULE ${PLUGIN}.cc )
  TARGET_LINK_LIBRARIES( ${PLUGIN} stage )
endforeach( PLUGIN )

# delete the "lib" prefix from the plugin libraries
SET_TARGET_PROPERTIES( ${PLUGINS} PROPERTIES PREFIX "" )

# need plaer's wavefront planning library for this one
#if( PLAYER_FOUND )  
#  link_directories( ${PLAYER_LIBRARY_DIRS} )
#  include_directories( ${PLAYER_INCLUDE_DIRS} )
#  target_link_libraries( fasr_plan "-lwavefront_standalone" )
#endif( PLAYER_FOUND )


# install in <prefix>/lib
install( TARGETS ${PLUGINS} DESTINATION lib)

