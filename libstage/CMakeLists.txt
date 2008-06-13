
add_library( stage SHARED
             stage.hh  # get headers into IDE projects
		file_manager.hh
		file_manager.cc
	     ancestor.cc 
	     block.cc 
	     canvas.cc 
		 camera.cc
	     gl.cc 
	     glcolorstack.cc 
	     model.cc 
	     model_blinkenlight.cc
	     model_blobfinder.cc 
	     model_callbacks.cc 
	     model_camera.cc 
	     model_fiducial.cc 
	     model_laser.cc 
	     model_load.cc 
	     model_position.cc 
	     model_props.cc 
	     model_ranger.cc 
	     resource.cc
	     stage.cc 
	     typetable.cc 
	     world.cc 
	     worldfile.cc 
	     worldgui.cc
		 texture_manager.cc
)

target_link_libraries( stage
		       ${GLIB_LIBRARIES}
		       ${OPENGL_LIBRARIES}
		       ${FLTK_LIBRARIES} 
		       ${APPLE_LIBRARIES}
		       ltdl
)

# causes the shared library to have a version number
set_target_properties( stage PROPERTIES
		       VERSION ${VERSION}
)


add_executable( stagebinary main.cc )

set_target_properties( stagebinary PROPERTIES
		       OUTPUT_NAME stage 
)


set_target_properties( stagebinary PROPERTIES
		       OUTPUT_NAME stage 
)

target_link_libraries( stagebinary
		       stage
) 

INSTALL(TARGETS stagebinary stage
	RUNTIME DESTINATION bin 
	LIBRARY DESTINATION lib
)

INSTALL(FILES stage.hh
        DESTINATION include/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR})
