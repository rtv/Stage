#include "lsp_test_proxy.hh"

using namespace lspTest;

const double Proxy::Delta = 0.01;

void Proxy::connect() {
	client = playerc_client_create( NULL, "localhost", 6665 );
	CPPUNIT_ASSERT( playerc_client_connect( client ) == 0 );
	CPPUNIT_ASSERT( playerc_client_datamode (client, PLAYERC_DATAMODE_PULL) == 0 );
	CPPUNIT_ASSERT( playerc_client_set_replace_rule (client, -1, -1, PLAYER_MSGTYPE_DATA, -1, 1) == 0 );
}

void Proxy::disconnect() {
	CPPUNIT_ASSERT( playerc_client_disconnect( client ) == 0 );
	playerc_client_destroy( client );
}
