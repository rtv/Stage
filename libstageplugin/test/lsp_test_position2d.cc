#include "lsp_test_position2d.hh"


using namespace lspTest;

void Position2D::setUp() {
	connect();
	posProxy = playerc_position2d_create( client, 0 );
	CPPUNIT_ASSERT( playerc_position2d_subscribe( posProxy, PLAYER_OPEN_MODE ) == 0 );
	
	// ignore initial messages on proxy subscription
	for ( int i = 0; i < 10; i++ )
		playerc_client_read( client );
}

void Position2D::tearDown() {
	CPPUNIT_ASSERT( playerc_position2d_unsubscribe( posProxy ) == 0 );
	playerc_position2d_destroy( posProxy );
	disconnect();
}

void Position2D::testGeom() {
	CPPUNIT_ASSERT( playerc_position2d_get_geom( posProxy ) == 0 );
	
	// values from pioneer.inc
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "geom pose (x)", -0.04, posProxy->pose[0], Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "geom pose (y)", 0, posProxy->pose[1], Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "geom pose (angle)", 0, posProxy->pose[2], Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "geom size (x)", 0.44, posProxy->size[0], Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "geom size (y)", 0.38, posProxy->size[1], Delta );
}

void Position2D::testData() {
	playerc_client_read( client );	
	
	// verify that we're getting new data
	posProxy->info.fresh = 0;
	playerc_client_read( client );
	CPPUNIT_ASSERT_MESSAGE( "laser updating", posProxy->info.fresh == 1 );
	
	CPPUNIT_ASSERT( posProxy->info.datatime > 0 );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "pose (x)", 0, posProxy->px, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "pose (y)", 0, posProxy->py, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "pose (angle)", 0, posProxy->pa, Delta );
	CPPUNIT_ASSERT( posProxy->stall == 0 );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "forward velocity", 0, posProxy->vx, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "horizontal velocity", 0, posProxy->vy, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "angular velocity", 0, posProxy->va, Delta );
}

void Position2D::testMove() {
	// Back the robot into the wall
	CPPUNIT_ASSERT( playerc_position2d_set_cmd_vel( posProxy, -3, 0, 0, 1 ) == 0 );
	// for 500ms, read and stop
	usleep( 500000 );
	playerc_client_read( client );
	CPPUNIT_ASSERT( playerc_position2d_set_cmd_vel( posProxy, 0, 0, 0, 1 ) == 0 );

	// Make sure it moved back
	CPPUNIT_ASSERT( posProxy->px < 0 );
	// Make sure it didn't turn or move sideways
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "pose (y)", 0, posProxy->py, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "pose (angle)", 0, posProxy->pa, Delta );
	// Make sure it's hitting the wall
	CPPUNIT_ASSERT( posProxy->stall == 1 );
	// Make sure the velocity is as specified
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "forward velocity", -3, posProxy->vx, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "horizontal velocity", 0, posProxy->vy, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "angular velocity", 0, posProxy->va, Delta );
	
	// Start turning left at 45 deg. / sec
	CPPUNIT_ASSERT( playerc_position2d_set_cmd_vel( posProxy, 0, 0, M_PI/2, 1 ) == 0 );
	// for 500ms, stop, and read
	usleep( 500000 );
	CPPUNIT_ASSERT( playerc_position2d_set_cmd_vel( posProxy, 0, 0, 0, 1 ) == 0 );
	usleep( 500000 );
	playerc_client_read( client );

	// Make sure odom still reads negative
	CPPUNIT_ASSERT( posProxy->px < 0 );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "pose (y)", 0, posProxy->py, Delta );
	// See if we turned in the right direction
	CPPUNIT_ASSERT( posProxy > 0 );
	// Shouldn't be stalled
	CPPUNIT_ASSERT_EQUAL( 0, posProxy->stall );
	// Should have stopped
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "forward velocity", 0, posProxy->vx, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "horizontal velocity", 0, posProxy->vy, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "angular velocity", 0, posProxy->va, Delta );
	
	// Save position and try position based movement toward start position
	double oldx = posProxy->px;
	double olda = posProxy->pa;
	CPPUNIT_ASSERT( playerc_position2d_set_cmd_pose( posProxy, 0, 0, 0, 1 ) == 0 );
	sleep( 5 );
	CPPUNIT_ASSERT( playerc_position2d_set_cmd_vel( posProxy, 0, 0, 0, 1 ) == 0 );
	usleep( 500000 );
	playerc_client_read( client );
	
	// Make sure we've made progress
	CPPUNIT_ASSERT( posProxy->px > oldx );
	CPPUNIT_ASSERT( posProxy->pa < olda );
	CPPUNIT_ASSERT( posProxy->stall == 0 );
	// Verify that we've stopped
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "forward velocity", 0, posProxy->vx, 0.1 );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "horizontal velocity", 0, posProxy->vy, 0.1 );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "angular velocity", 0, posProxy->va, 0.1 );
	
	// Reset odometer and check position
	playerc_client_read( client );
	CPPUNIT_ASSERT( playerc_position2d_set_odom( posProxy, 0, 0, 0 ) == 0 );
	usleep( 500000 );
	playerc_client_read( client );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "pose (x)", 0, posProxy->px, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "pose (y)", 0, posProxy->py, Delta );
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "pose (angle)", 0, posProxy->pa, Delta );	
}
