MESSAGE( STATUS "Building Player plugin" )

link_directories( ${PLAYER_LIBRARY_DIRS} )
include_directories( ${PLAYER_INCLUDE_DIRS} )

set( stagepluginSrcs p_driver.h
	     p_driver.cc
	     p_blobfinder.cc
	     p_simulation.cc
	     p_laser.cc
	     p_fiducial.cc
	     p_position.cc
	     p_sonar.cc
	     p_speech.cc
	     stg_time.cc
)

add_library( stageplugin MODULE ${stagepluginSrcs} )

#	     p_graphics3d.cc


target_link_libraries( stageplugin
		       stage
		       ${PLAYER_LIBRARIES}
		       playerutils
		       ${OPENGL_LIBRARIES}

)

IF (BUILD_LSPTEST)
	ADD_SUBDIRECTORY(test)
ENDIF (BUILD_LSPTEST)

INSTALL(TARGETS stageplugin
	LIBRARY DESTINATION lib
)

