SET( PLUGINS lasernoise source fasr sink swarmbenchmark )

ADD_LIBRARY( lasernoise MODULE lasernoise.cc )
ADD_LIBRARY( source MODULE source.cc )
ADD_LIBRARY( fasr MODULE fasr.cc )
ADD_LIBRARY( sink MODULE sink.cc )
ADD_LIBRARY( swarmbenchmark MODULE swarmbenchmark.cc )

# link libstage to each plugin
foreach( PLUGIN ${PLUGINS} )
  TARGET_LINK_LIBRARIES( ${PLUGIN} stage )
endforeach( PLUGIN )

# delete the "lib" prefix from the plugin libraries
SET_TARGET_PROPERTIES( ${PLUGINS} PROPERTIES PREFIX "" )

# install in <prefix>/lib
INSTALL( TARGETS ${PLUGINS} DESTINATION lib)
