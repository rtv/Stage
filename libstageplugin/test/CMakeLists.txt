MESSAGE( STATUS "Building Player plugin tests" )

pkg_search_module( PLAYERC REQUIRED playerc )

SET( lspTestSrcs
	lsp_test.cc
)

add_executable( lsp_test ${lspTestSrcs} )
target_link_libraries( lsp_test PLAYERC )
