#ifndef _LSP_POSITION2D_TEST_H_
#define _LSP_POSITION2D_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <libplayerc/playerc.h>

#include "lsp_test_proxy.hh"

namespace lspTest {
	class Position2D : public Proxy
	{
		CPPUNIT_TEST_SUB_SUITE( Position2D, Proxy );
		CPPUNIT_TEST( testGeom );
		CPPUNIT_TEST( testData );
		CPPUNIT_TEST( testMove );
		CPPUNIT_TEST_SUITE_END();
		
	protected:
		playerc_position2d_t* posProxy;
		
		void testGeom();
		void testData();
		void testMove();
		
	public:
		void setUp();
		void tearDown();
	};
};

CPPUNIT_TEST_SUITE_REGISTRATION( lspTest::Position2D );

#endif
