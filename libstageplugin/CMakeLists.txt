MESSAGE( STATUS "Configuring libstageplugin" )

link_directories( ${PLAYER_LIBRARY_DIRS} )
include_directories( ${PLAYER_INCLUDE_DIRS} )

set( stagepluginSrcs 
	     p_driver.h
	     p_driver.cc
	     p_actarray.cc
	     p_blobfinder.cc
			 p_gripper.cc
	     p_simulation.cc
	     p_fiducial.cc
	     p_position.cc
	     p_ranger.cc			 
	     p_speech.cc
	     p_graphics.cc
	     stg_time.cc
)

add_library( stageplugin MODULE ${stagepluginSrcs} )

target_link_libraries( stageplugin
		       stage
		       ${PLAYER_LIBRARIES}
		       ${OPENGL_LIBRARIES}
)

set_source_files_properties( ${stagepluginSrcs} PROPERTIES COMPILE_FLAGS "${FLTK_CFLAGS}" )

# delete the "lib" prefix from the plugin
SET_TARGET_PROPERTIES( stageplugin PROPERTIES 
                       PREFIX ""
                       LINK_FLAGS "${FLTK_LDFLAGS}"
 )

IF (BUILD_LSPTEST)
	ADD_SUBDIRECTORY(test)
ENDIF (BUILD_LSPTEST)

INSTALL(TARGETS stageplugin
	LIBRARY DESTINATION ${PROJECT_LIB_DIR}
)

