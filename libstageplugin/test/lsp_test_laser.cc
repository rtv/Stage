#include "lsp_test_laser.hh"

using namespace lspTest;

void Laser::setUp() {
	connect();
	laserProxy = playerc_laser_create( client, 0 );
	CPPUNIT_ASSERT( playerc_laser_subscribe( laserProxy, PLAYER_OPEN_MODE ) == 0 );
}


void Laser::tearDown() {
	CPPUNIT_ASSERT( playerc_laser_unsubscribe( laserProxy ) == 0 );
	playerc_laser_destroy( laserProxy );
	disconnect();
}

void Laser::testConfig() {
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

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "min scan angle", min, min2, DELTA );	
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "max scan angle", max, max2, DELTA );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "angular resolution", res, res2, DELTA );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "range resolution", range_res, range_res2, DELTA );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "scan freq", freq, freq2, DELTA );
	CPPUNIT_ASSERT_EQUAL_MESSAGE( "intensity", intensity, intensity2 );
}

void Laser::testData() {
	
}