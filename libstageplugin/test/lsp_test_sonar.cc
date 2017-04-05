#include "lsp_test_sonar.hh"

using namespace lspTest;

void Sonar::setUp()
{
  connect();
  sonarProxy = playerc_sonar_create(client, 0);
  CPPUNIT_ASSERT(playerc_sonar_subscribe(sonarProxy, PLAYER_OPEN_MODE) == 0);
}

void Sonar::tearDown()
{
  CPPUNIT_ASSERT(playerc_sonar_unsubscribe(sonarProxy) == 0);
  playerc_sonar_destroy(sonarProxy);
  disconnect();
}

void Sonar::testGeom()
{
  CPPUNIT_ASSERT(playerc_sonar_get_geom(sonarProxy) == 0);

  // values from pioneer.inc
  CPPUNIT_ASSERT(sonarProxy->pose_count == 16);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose[0] (x)", 0.075, sonarProxy->poses[0].px, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose[0] (y)", 0.130, sonarProxy->poses[0].py, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose[0] (z)", 0, sonarProxy->poses[0].pz,
                                       Delta); // currently unused
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose[0] (angle)", M_PI / 2, sonarProxy->poses[0].pyaw,
                                       Delta);
}

void Sonar::testData()
{
  playerc_client_read(client);

  // verify that we're getting new data
  sonarProxy->info.fresh = 0;
  playerc_client_read(client);
  CPPUNIT_ASSERT_MESSAGE("sonar updating", sonarProxy->info.fresh == 1);

  CPPUNIT_ASSERT(sonarProxy->info.datatime > 0);
  CPPUNIT_ASSERT(sonarProxy->scan_count == 16); // from pioneer.inc, actually sensor_count

  // sanity check
  for (int i = 0; i < sonarProxy->scan_count; i++) {
    CPPUNIT_ASSERT(sonarProxy->scan[i] >= 0);
    //		printf(" range[%d]: %6.3f ]\n", i, sonarProxy->scan[i]);
  }
}
