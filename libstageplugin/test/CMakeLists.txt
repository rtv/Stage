MESSAGE( STATUS "Building Player plugin tests" )

pkg_search_module( PLAYERC REQUIRED playerc )
pkg_search_module( CPPUNIT REQUIRED cppunit )

include_directories(
	${CPPUNIT_INCLUDE_PATH}
)

link_directories(
	${CPPUNIT_LIBRARY_DIRS}
)

SET( lspTestSrcs
	lsp_test.cc
	lsp_test_proxy.cc
	lsp_test_proxy.hh
	lsp_test_laser.cc
	lsp_test_laser.hh
	lsp_test_speech.cc
	lsp_test_speech.hh
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
)
