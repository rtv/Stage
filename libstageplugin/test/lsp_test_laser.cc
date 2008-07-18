#include "lsp_test_laser.hh"



void LSPLaserTest::setUp() {
	client = playerc_client_create( NULL, "localhost", 6665 );
	
//	int connectError = playerc_client_connect( client );
	CPPUNIT_ASSERT( playerc_client_connect( client ) == 0 );
	
	laserProxy = playerc_laser_create( client, 0 );
	
	CPPUNIT_ASSERT( playerc_laser_subscribe( laserProxy, PLAYER_OPEN_MODE ) == 0 );
}


void LSPLaserTest::tearDown() {
	CPPUNIT_ASSERT( playerc_laser_unsubscribe( laserProxy ) == 0 );
	playerc_laser_destroy( laserProxy );
	
	CPPUNIT_ASSERT( playerc_client_disconnect( client ) == 0 );
	playerc_client_destroy( client );
}

void LSPLaserTest::testConfig() {
	const double DELTA = 0.01;
	
	double min = -M_PI/2;
	double max = +M_PI/2;
	double res = 100;
	double range_res = 1;
	unsigned char intensity = 1;
	double freq = 10;
	
	CPPUNIT_ASSERT( playerc_laser_set_config( laserProxy, min, max, res, range_res, intensity, freq ) == 0 );
	
	double min2, max2, res2, range_res2, freq2;
	unsigned char intensity2 = 1;
	CPPUNIT_ASSERT( playerc_laser_get_config( laserProxy, &min2, &max2, &res2, &range_res2, &intensity2, &freq2 ) == 0 );

	CPPUNIT_ASSERT_DOUBLES_EQUAL( min2, min, DELTA );	
	CPPUNIT_ASSERT_DOUBLES_EQUAL( max2, max, DELTA );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( res2, res, DELTA );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( range_res2, range_res, DELTA );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( freq2, freq, DELTA );
	CPPUNIT_ASSERT( intensity == intensity2 );
}

void LSPLaserTest::testData() {
	
}