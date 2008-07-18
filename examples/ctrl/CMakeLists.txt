
SET( PLUGINS
     fasr
     lasernoise
     sink
     source
     wander
)


# create a library module for each plugin and link libstage to each

foreach( PLUGIN ${PLUGINS} )
  ADD_LIBRARY( ${PLUGIN} MODULE ${PLUGIN}.cc )
  TARGET_LINK_LIBRARIES( ${PLUGIN} stage )
endforeach( PLUGIN )

# delete the "lib" prefix from the plugin libraries
SET_TARGET_PROPERTIES( ${PLUGINS} PROPERTIES PREFIX "" )

# install in <prefix>/lib
INSTALL( TARGETS ${PLUGINS} DESTINATION lib)
