
link_directories(${GLIB_LIBRARY_DIRS}
		${FLTK_LIBRARY_DIRS}
		${OPENGL_LIBRARY_DIRS}
)


add_library( stage SHARED
	     ancestor.cc 
	     block.cc 
	     canvas.cc 
	     gl.cc 
	     glcolorstack.cc 
	     model.cc 
	     model_callbacks.cc 
	     model_fiducial.cc 
	     model_laser.cc 
	     model_load.cc 
	     model_position.cc 
	     model_ranger.cc 
	     model_blobfinder.cc 
	     model_props.cc 
	     stage.cc 
	     typetable.cc 
	     world.cc 
	     worldfile.cc 
	     worldgui.cc 
)

add_executable( stage main.cc ) 



target_link_libraries( stage
		       ${GLIB_LIBRARIES}
		       ${FLTK_LIBRARIES} 
		       ${OPENGL_LIBRARIES}
		       ltdl
)

