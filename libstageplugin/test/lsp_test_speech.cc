#include "lsp_test_speech.hh"

using namespace lspTest;

void Speech::setUp() {
	connect();
	speechProxy = playerc_speech_create( client, 0 );
	CPPUNIT_ASSERT( playerc_speech_subscribe( speechProxy, PLAYER_OPEN_MODE ) == 0 );
}

void Speech::tearDown() {
	CPPUNIT_ASSERT( playerc_speech_unsubscribe( speechProxy ) == 0 );
	playerc_speech_destroy( speechProxy );
	disconnect();
}

void Speech::testSay1() {
	CPPUNIT_ASSERT( playerc_speech_say( speechProxy, "Test" ) == 0 );
}
