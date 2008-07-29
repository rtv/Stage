message( STATUS "Building Player plugin tests" )

pkg_search_module( CPPUNIT REQUIRED cppunit )
pkg_search_module( PLAYERC REQUIRED playerc )

include_directories(
	${CPPUNIT_INCLUDE_DIRS}
	${PLAYERC_INCLUDE_DIRS}
)

link_directories( 
	${CPPUNIT_LIBRARY_DIRS}
	${PLAYERC_LIBRARY_DIRS}
)

set( lspTestSrcs
	lsp_test.cc
	lsp_test_proxy.cc
	lsp_test_proxy.hh
	lsp_test_blobfinder.cc
	lsp_test_blobfinder.hh
	lsp_test_simulation.cc
	lsp_test_simulation.hh
	lsp_test_speech.cc
	lsp_test_speech.hh
	lsp_test_laser.cc
	lsp_test_laser.hh
	lsp_test_sonar.cc
	lsp_test_sonar.hh
	lsp_test_fiducial.cc
	lsp_test_fiducial.hh
	lsp_test_position2d.cc
	lsp_test_position2d.hh
)

add_executable( lsp_test ${lspTestSrcs} )

target_link_libraries( lsp_test
	${PLAYERC_LIBRARIES}
	${CPPUNIT_LIBRARIES}
	dl
)
