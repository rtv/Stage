#ifndef _LSP_FIDUCIAL_TEST_H_
#define _LSP_FIDUCIAL_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <libplayerc/playerc.h>

#include "lsp_test_proxy.hh"

namespace lspTest {
	class Fiducial : public Proxy
		{
			CPPUNIT_TEST_SUITE( Fiducial );
			CPPUNIT_TEST( testGeom );
			CPPUNIT_TEST( testData );
			CPPUNIT_TEST_SUITE_END();
			
		protected:
			playerc_fiducial_t* fiducialProxy;
			
			void testConfig();
			void testGeom();
			void testData();
			
		public:
			void setUp();
			void tearDown();
		};
};

CPPUNIT_TEST_SUITE_REGISTRATION( lspTest::Fiducial );

#endif
