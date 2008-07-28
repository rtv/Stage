#ifndef _LSP_BLOBFINDER_TEST_H_
#define _LSP_BLOBFINDER_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <libplayerc/playerc.h>

#include "lsp_test_proxy.hh"

namespace lspTest {
	class Blobfinder : public Proxy
		{
			CPPUNIT_TEST_SUITE( Blobfinder );
			CPPUNIT_TEST( testData );
			CPPUNIT_TEST_SUITE_END();
			
		protected:
			playerc_blobfinder_t* blobProxy;
			
			void testConfig();
			void testGeom();
			void testData();
			
		public:
			void setUp();
			void tearDown();
		};
};

CPPUNIT_TEST_SUITE_REGISTRATION( lspTest::Blobfinder );

#endif
