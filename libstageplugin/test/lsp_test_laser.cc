#include "lsp_test_laser.hh"

using namespace lspTest;

const int Laser::Samples = 361;

void Laser::setUp()
{
  connect();
  laserProxy = playerc_laser_create(client, 0);
  CPPUNIT_ASSERT(playerc_laser_subscribe(laserProxy, PLAYER_OPEN_MODE) == 0);
}

void Laser::tearDown()
{
  CPPUNIT_ASSERT(playerc_laser_unsubscribe(laserProxy) == 0);
  playerc_laser_destroy(laserProxy);
  disconnect();
}

void Laser::testConfig()
{
  double min = -M_PI / 2;
  double max = +M_PI / 2;
  double res = M_PI / Samples; // sick laser default
  double range_res = 1; // not being used by stage
  unsigned char intensity = 1; // not being used by stage
  double freq = 10; // 10Hz / 100ms (stage default)

  CPPUNIT_ASSERT(playerc_laser_set_config(laserProxy, min, max, res, range_res, intensity, freq)
                 == 0);

  double min2, max2, res2, range_res2, freq2;
  unsigned char intensity2 = 1;
  CPPUNIT_ASSERT(
      playerc_laser_get_config(laserProxy, &min2, &max2, &res2, &range_res2, &intensity2, &freq2)
      == 0);

  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("min scan angle", min, min2, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("max scan angle", max, max2, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("angular resolution", res, res2, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("range resolution", range_res, range_res2, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("scan freq", freq, freq2, Delta);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("intensity", intensity, intensity2);
}

void Laser::testGeom()
{
  CPPUNIT_ASSERT(playerc_laser_get_geom(laserProxy) == 0);

  // values from lsp_test.world and sick.inc
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (x)", 0.03, laserProxy->pose[0], Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (y)", 0, laserProxy->pose[1], Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (angle)", 0, laserProxy->pose[2], Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("size (x)", 0.156, laserProxy->size[0], Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("size (y)", 0.155, laserProxy->size[1], Delta);
}

void Laser::testData()
{
  playerc_client_read(client);

  // verify that we're getting new data
  laserProxy->info.fresh = 0;
  playerc_client_read(client);
  CPPUNIT_ASSERT_MESSAGE("laser updating", laserProxy->info.fresh == 1);

  CPPUNIT_ASSERT(laserProxy->info.datatime > 0);
  CPPUNIT_ASSERT(laserProxy->scan_count == Samples);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("min scan angle", -M_PI / 2, laserProxy->scan[0][1], Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("max scan angle", M_PI / 2, laserProxy->scan[Samples - 1][1],
                                       Delta);

  for (int i = 0; i < laserProxy->scan_count; i++) {
    double distance = laserProxy->scan[i][0];

    // check range of each sample is within max and min
    CPPUNIT_ASSERT(distance <= laserProxy->max_range);
    CPPUNIT_ASSERT(distance >= laserProxy->min_right);

    //        printf("[%6.3f, %6.3f ] \n", laserProxy->scan[i][0], laserProxy->scan[i][1]);
  }
}
