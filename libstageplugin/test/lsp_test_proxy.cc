#include "lsp_test_proxy.hh"

#include <stdio.h>

using namespace lspTest;

void Proxy::connect() {
	client = playerc_client_create( NULL, "localhost", 6665 );
	CPPUNIT_ASSERT( playerc_client_connect( client ) == 0 );
}

void Proxy::disconnect() {
	CPPUNIT_ASSERT( playerc_client_disconnect( client ) == 0 );
	playerc_client_destroy( client );
}