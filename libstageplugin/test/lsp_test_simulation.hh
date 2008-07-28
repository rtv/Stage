#ifndef _LSP_SIMULATION_TEST_H_
#define _LSP_SIMULATION_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <libplayerc/playerc.h>

#include "lsp_test_proxy.hh"

namespace lspTest {
	class Simulation : public Proxy
		{
			CPPUNIT_TEST_SUB_SUITE( Simulation, Proxy );
			CPPUNIT_TEST( testPose2D );
			CPPUNIT_TEST( testPose3D );
			CPPUNIT_TEST( testProperties );
			CPPUNIT_TEST_SUITE_END();
			
		protected:
			playerc_simulation_t* simProxy;
			
			void testPose2D();
			void testPose3D();
			void testProperties();
			
		public:
			void setUp();
			void tearDown();
		};
};

CPPUNIT_TEST_SUITE_REGISTRATION( lspTest::Simulation );

#endif
