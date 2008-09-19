MESSAGE( STATUS "Configuring libstage" )

set( stageSrcs ancestor.cc
	block.cc
	camera.cc
	canvas.cc
	file_manager.cc
	file_manager.hh
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
	option.cc
	option.hh
	options_dlg.cc
	options_dlg.hh
	resource.cc
	stage.cc
	stage.hh
	texture_manager.cc
	typetable.cc
	world.cc
	worldfile.cc
	worldgui.cc
)

add_library(stage SHARED ${stageSrcs})

target_link_libraries( stage
		       ${GLIB_LIBRARIES}
		       ${OPENGL_LIBRARIES}
 				 ${LTDL_LIB}
)

# When linking stage, pass LDFLAGS from fltk-config
#   use quotes to prevent spaces being parsed as a list of properties
set_target_properties( stage PROPERTIES LINK_FLAGS "${FLTK_LDFLAGS}" )

# When compiling stageSrcs, pass CFLAGS from fltk-config
set_source_files_properties( ${stageSrcs} PROPERTIES COMPILE_FLAGS "${FLTK_CFLAGS}" )

# for the config.h
include_directories(${PROJECT_BINARY_DIR})

# causes the shared library to have a version number
set_target_properties( stage PROPERTIES
		       VERSION ${VERSION}
)

set( stagebinarySrcs main.cc )

add_executable( stagebinary ${stagebinarySrcs} )

# When compiling stagebinarySrcs, pass CFLAGS from fltk-config
set_source_files_properties( ${stagebinarySrcs} PROPERTIES COMPILE_FLAGS "${FLTK_CFLAGS}" )

set_target_properties( stagebinary PROPERTIES
		       OUTPUT_NAME stage
             LINK_FLAGS "${FLTK_LDFLAGS}"	  
)

target_link_libraries( stagebinary stage )

set( stagetestSrcs test.cc )

add_executable( stagetest ${stagetestSrcs} )
target_link_libraries( stagetest stage )


INSTALL(TARGETS stagebinary stage
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
)

INSTALL(FILES stage.hh option.hh worldfile.hh
        DESTINATION include/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR})

ADD_TEST( test1 ${EXECUTABLE_OUTPUT_PATH}stagetest )
