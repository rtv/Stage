#include "lsp_test_blobfinder.hh"

using namespace lspTest;

void Blobfinder::setUp() {
	connect();
	blobProxy = playerc_blobfinder_create( client, 0 );
	CPPUNIT_ASSERT( playerc_blobfinder_subscribe( blobProxy, PLAYER_OPEN_MODE ) == 0 );
}


void Blobfinder::tearDown() {
	CPPUNIT_ASSERT( playerc_blobfinder_unsubscribe( blobProxy ) == 0 );
	playerc_blobfinder_destroy( blobProxy );
	disconnect();
}

void Blobfinder::testData() {
	playerc_client_read( client );
	
	// verify that we're getting new data
	blobProxy->info.fresh = 0;
	playerc_client_read( client );
	CPPUNIT_ASSERT_MESSAGE( "blobfinder updating", blobProxy->info.fresh == 1 );
	
	CPPUNIT_ASSERT( blobProxy->info.datatime > 0 );

	//printf("\ncolor0: 0x%X, color1: 0x%X\n", blobProxy->blobs[0].color, blobProxy->blobs[1].color);
	
	// check stage defaults
	CPPUNIT_ASSERT( blobProxy->width == 80 );
	CPPUNIT_ASSERT( blobProxy->height == 60 );
	
	// Make sure we see three blobs: wall, robot, wall
	CPPUNIT_ASSERT_EQUAL_MESSAGE( "blobs_count", (unsigned int)3, blobProxy->blobs_count ); // lsp_test.world
	// sanity checks
	CPPUNIT_ASSERT( blobProxy->blobs[1].range > 0 );
	CPPUNIT_ASSERT( blobProxy->blobs[1].area > 0 );
	CPPUNIT_ASSERT( blobProxy->blobs[1].y > 0 );
	CPPUNIT_ASSERT( blobProxy->blobs[1].left < blobProxy->blobs[1].right );
	CPPUNIT_ASSERT( blobProxy->blobs[1].top < blobProxy->blobs[1].bottom );
	
	// robot should be red
	CPPUNIT_ASSERT( blobProxy->blobs[1].color == 0xFFFF0000 );
	
	// robot should be closer
	CPPUNIT_ASSERT( blobProxy->blobs[0].range > blobProxy->blobs[1].range );
	
	// robot should be towards the right
	uint32_t robotCenter = blobProxy->blobs[1].x;
	uint32_t imageCenter = blobProxy->width / 2;
	//printf(" robotCenter: %d, imageCenter: %d\n", robotCenter, imageCenter);
	CPPUNIT_ASSERT( robotCenter > imageCenter );
	
	
	
//	CPPUNIT_ASSERT_EQUAL_MESSAGE( "id", (unsigned int)2, blobProxy->blobs[0].id );

}
