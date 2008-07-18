#ifndef _LSP_LASER_TEST_H_
#define _LSP_LASER_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <libplayerc/playerc.h>

class LSPLaserTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( LSPLaserTest );
	CPPUNIT_TEST( testConfig );
	CPPUNIT_TEST( testData );
	CPPUNIT_TEST_SUITE_END();
	
protected:
	playerc_laser_t* laserProxy;
	playerc_client_t* client;
	
	void testConfig();
	void testData();
	
public:
	void setUp();
	void tearDown();
};

CPPUNIT_TEST_SUITE_REGISTRATION( LSPLaserTest );

#endif