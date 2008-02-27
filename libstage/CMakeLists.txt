
include(FindPkgConfig)
pkg_check_modules( GLIB gdk-pixbuf-2.0 )

# hack to find FLTK on OS X using MacPorts
SET(FLTK_INCLUDE_DIR /opt/local/include)

find_package(OPENGL REQUIRED)
find_package(FLTK REQUIRED)

MESSAGE("hello")
MESSAGE("FL INC ${FLTK_INCLUDE_DIR}")
MESSAGE("FL LIBS ${FLTK_LIBRARIES}")
MESSAGE("GL INC ${OPENGL_INCLUDE_DIR}")
MESSAGE("GL LIBS ${OPENGL_LIBRARIES}")

include_directories( .
		     .. 
		     ../replace 
		     ${GLIB_INCLUDE_DIRS} 
		     ${FLTK_INCLUDE_DIR}
		     ${OPENGL_INCLUDE_DIR}
)


link_directories(${GLIB_LIBRARY_DIRS}
		${FLTK_LIBRARY_DIRS}
		${OPENGL_LIBRARY_DIRS}
)

add_executable(stage 
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
		     main.cc
)



target_link_libraries( stage
		       ${GLIB_LIBRARIES}
		       ${FLTK_LIBRARIES} 
		       ${OPENGL_LIBRARIES}
		       ltdl
)

