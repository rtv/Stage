MESSAGE( STATUS "Configuring webstage" )

add_executable( webstage webstage.cc )

target_link_libraries( webstage  ${WEBSIM_LIBRARIES} stage )

INSTALL(TARGETS webstage
	RUNTIME DESTINATION bin
)

