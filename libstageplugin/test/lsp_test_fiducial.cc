#include "lsp_test_fiducial.hh"

using namespace lspTest;

void Fiducial::setUp()
{
  connect();
  fiducialProxy = playerc_fiducial_create(client, 0);
  CPPUNIT_ASSERT(playerc_fiducial_subscribe(fiducialProxy, PLAYER_OPEN_MODE) == 0);
}

void Fiducial::tearDown()
{
  CPPUNIT_ASSERT(playerc_fiducial_unsubscribe(fiducialProxy) == 0);
  playerc_fiducial_destroy(fiducialProxy);
  disconnect();
}

void Fiducial::testGeom()
{
  CPPUNIT_ASSERT(playerc_fiducial_get_geom(fiducialProxy) == 0);

  // values from lsp_test.world
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (x)", -0.15, fiducialProxy->fiducial_geom.pose.px,
                                       Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (y)", 0, fiducialProxy->fiducial_geom.pose.py, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (z)", 0, fiducialProxy->fiducial_geom.pose.pz, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (pitch)", 0, fiducialProxy->fiducial_geom.pose.ppitch,
                                       Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (roll)", 0, fiducialProxy->fiducial_geom.pose.proll,
                                       Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (yaw)", 0, fiducialProxy->fiducial_geom.pose.pyaw,
                                       Delta);
}

void Fiducial::testData()
{
  playerc_client_read(client);

  // verify that we're getting new data
  fiducialProxy->info.fresh = 0;
  playerc_client_read(client);
  CPPUNIT_ASSERT_MESSAGE("fiducial updating", fiducialProxy->info.fresh == 1);

  CPPUNIT_ASSERT(fiducialProxy->info.datatime > 0);

  // Make sure we see exactly 1 robot with ID 2
  CPPUNIT_ASSERT_EQUAL_MESSAGE("fiducials_count", 1,
                               fiducialProxy->fiducials_count); // lsp_test.world
  CPPUNIT_ASSERT(fiducialProxy->fiducials[0].id == 2);

  //	printf("\nfiducials_count: %d\n", fiducialProxy->fiducials_count );
  //	for ( int i = 0; i < fiducialProxy->fiducials_count; i++ ) {
  //		printf( "fiducial return: %d @ [ %6.4f %6.4f %6.4f ]\n",
  //			    fiducialProxy->fiducials[i].id,
  //			    fiducialProxy->fiducials[i].pose.px,
  //			    fiducialProxy->fiducials[i].pose.py,
  //			    fiducialProxy->fiducials[i].pose.pyaw );
  //	}
}
