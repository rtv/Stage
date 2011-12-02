MESSAGE( STATUS "Configuring libstage" )

# for config.h
include_directories(${PROJECT_BINARY_DIR})

set( stageSrcs    
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
	options_dlg.cc
	options_dlg.hh
	vis_strip.cc			
	worldgui.cc 
	ancestor.cc
)

#	model_getset.cc
#	model_load.cc

#set_source_files_properties( ${stageSrcs} PROPERTIES COMPILE_FLAGS" )
 
add_library(stage SHARED ${stageSrcs})

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

target_link_libraries( stage 
                       ${LTDL_LIB} 
                       ${JPEG_LIBRARIES} 
                       ${PNG_LIBRARIES}
                       ${FLTK_LIBRARIES}
)

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

target_link_libraries( stagebinary stage )

IF(PROJECT_OS_LINUX)
  target_link_libraries( stagebinary stage pthread )
ENDIF(PROJECT_OS_LINUX)

INSTALL(TARGETS stagebinary stage
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${PROJECT_LIB_DIR}
)

INSTALL(FILES stage.hh 
        DESTINATION include/${PROJECT_NAME}-${APIVERSION})

