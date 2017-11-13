MESSAGE( STATUS "Configuring libstage" )

# for config.h
include_directories(${PROJECT_BINARY_DIR})

set(stageSrcs
	block.cc
	blockgroup.cc
	camera.cc
	color.cc
	file_manager.cc
	file_manager.hh
	gl.cc
	logentry.cc
	model.cc
	model_actuator.cc
	model_blinkenlight.cc
	model_blobfinder.cc
	model_bumper.cc
	model_callbacks.cc
	model_camera.cc
	model_draw.cc
	model_fiducial.cc
	model_gripper.cc
	model_lightindicator.cc
	model_position.cc
	model_ranger.cc
	option.cc
	powerpack.cc
	region.cc
	stage.cc
	stage.hh
	texture_manager.cc
	typetable.cc
	world.cc
	worldfile.cc
	canvas.cc
	vis_strip.cc
	ancestor.cc
	image.cc
)

# Core library. Contains simulation code and most part of platform-independent rendering code
add_library(stage_core SHARED ${stageSrcs})
set_target_properties( stage_core PROPERTIES
		       VERSION ${VERSION}
#           LINK_FLAGS "${FLTK_LDFLAGS}"  
)
target_link_libraries( stage_core ${OPENGL_LIBRARIES} ${LTDL_LIB} ${JPEG_LIBRARIES} ${PNG_LIBRARIES})

# Headless binary version. Does not need FLTK, but stil references a lot of OpenGL
add_executable( stagebinary_headless main_headless.cc )
set_target_properties( stagebinary_headless PROPERTIES OUTPUT_NAME stage_headless )
target_link_libraries( stagebinary_headless stage_core )

INSTALL(TARGETS stagebinary_headless stage_core
	RUNTIME DESTINATION bin LIBRARY DESTINATION ${PROJECT_LIB_DIR}
)

INSTALL(FILES stage.hh DESTINATION include/${PROJECT_NAME}-${APIVERSION})

# GUI part of the stage
if(FLTK_FOUND AND BUILD_GUI)
	add_library(stage SHARED canvas_fltk.cc options_dlg.cc menu_manager_fltk.cc worldgui.cc)
	
	# if fltk-config didn't bring along the OpenGL dependencies (eg. on
	# Debian/Ubuntu), add them explicity 
	IF (NOT(${FLTK_LDFLAGS} MATCHES "-lGL"))
	  target_link_libraries( stage ${OPENGL_LIBRARIES}) 
	ENDIF (NOT(${FLTK_LDFLAGS} MATCHES "-lGL"))
	
	# causes the shared library to have a version number
	set_target_properties( stage PROPERTIES
			       VERSION ${VERSION}
	#           LINK_FLAGS "${FLTK_LDFLAGS}"  
	)
	
	target_link_libraries( stage stage_core ${FLTK_LIBRARIES})
	
	set( stagebinarySrcs main.cc )
	set_source_files_properties( ${stagebinarySrcs} PROPERTIES COMPILE_FLAGS "${FLTK_CFLAGS}" )
	
	# TODO: build an app bundle on OS X
	#set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
	#set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.playerstage.stage")
	#set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} Version ${VERSION}")
	#set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
	#set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION})
	#set(MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION})
	#set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2010. All Rights Reserved.")
	# add_executable( stagebinary MACOSX_BUNDLE ${stagebinarySrcs} )
	
	add_executable( stagebinary ${stagebinarySrcs} )
	set_target_properties( stagebinary PROPERTIES OUTPUT_NAME stage )
	
	# Apple seems to have trouble when libstage and stagebinary are linked against FLTK
	# Newer Linux distributions won't allow stagebinary to inherit libstage's links to fltk, so we need
	# to explicitly link on Linux
	target_link_libraries( stagebinary stage_core stage )
	
	IF(PROJECT_OS_LINUX)
	  target_link_libraries( stagebinary stage pthread )
	ENDIF(PROJECT_OS_LINUX)
	
	INSTALL(TARGETS stagebinary stage RUNTIME DESTINATION bin LIBRARY DESTINATION ${PROJECT_LIB_DIR})
endif(FLTK_FOUND AND BUILD_GUI)

