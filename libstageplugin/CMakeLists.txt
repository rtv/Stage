
link_directories( ${PLAYER_LIBDIR} )
include_directories( ${PLAYER_INCLUDE_DIRS})

add_library( stageplugin MODULE
	     p_driver.cc
	     p_simulation.cc
	     p_laser.cc
	     p_position.cc
	     p_sonar.cc
	     stg_time.cc
)

#	     p_graphics3d.cc
#	     p_fiducial.cc
#	     p_blobfinder.cc


target_link_libraries( stageplugin
		       stage
		       ${PLAYER_LIBRARIES}
		       playerutils
		       ${OPENGL_LIBRARIES}
		       
)

INSTALL(TARGETS stageplugin
	LIBRARY DESTINATION lib
)

