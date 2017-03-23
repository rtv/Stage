#include "lsp_test_simulation.hh"

using namespace lspTest;

void Simulation::setUp()
{
  connect();
  simProxy = playerc_simulation_create(client, 0);
  CPPUNIT_ASSERT(playerc_simulation_subscribe(simProxy, PLAYER_OPEN_MODE) == 0);
}

void Simulation::tearDown()
{
  CPPUNIT_ASSERT(playerc_simulation_unsubscribe(simProxy) == 0);
  playerc_simulation_destroy(simProxy);
  disconnect();
}

void Simulation::testPose2D()
{
  double x, y, a;
  double x2, y2, a2;

  // See if the robot "r1" is where it should be according to lsp_test.world
  CPPUNIT_ASSERT(playerc_simulation_get_pose2d(simProxy, "r1", &x, &y, &a) == 0);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (x)", -4.19, x, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (y)", -5.71, y, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (a)", 3 * M_PI / 4, a, Delta);

  // Set pose to [ 0, 0, 0 ] and verify
  CPPUNIT_ASSERT(playerc_simulation_set_pose2d(simProxy, "r1", 0, 0, 0) == 0);
  CPPUNIT_ASSERT(playerc_simulation_get_pose2d(simProxy, "r1", &x2, &y2, &a2) == 0);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (x)", 0, x2, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (y)", 0, y2, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (a)", 0, a2, Delta);

  // Return the robot to its starting point
  CPPUNIT_ASSERT(playerc_simulation_set_pose2d(simProxy, "r1", x, y, a) == 0);
}

void Simulation::testPose3D()
{
  double x, y, z, roll, pitch, yaw, time;
  double x2, y2, z2, roll2, pitch2, yaw2, time2;

  // See if the robot "r1" is where it should be according to lsp_test.world
  CPPUNIT_ASSERT(
      playerc_simulation_get_pose3d(simProxy, "r1", &x, &y, &z, &roll, &pitch, &yaw, &time) == 0);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (x)", -4.19, x, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (y)", -5.71, y, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (z)", 0, z, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (roll)", 0, roll, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (pitch)", 0, pitch, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (yaw)", 3 * M_PI / 4, yaw, Delta);
  CPPUNIT_ASSERT(time > 0);

  // Set pose to [ 0, 0, 0.5, M_PI/4, M_PI/4, M_PI/4 ] and verify
  CPPUNIT_ASSERT(
      playerc_simulation_set_pose3d(simProxy, "r1", 0, 0, 0.5, M_PI / 4, M_PI / 4, M_PI / 4) == 0);
  CPPUNIT_ASSERT(
      playerc_simulation_get_pose3d(simProxy, "r1", &x2, &y2, &z2, &roll2, &pitch2, &yaw2, &time2)
      == 0);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (x)", 0, x2, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (y)", 0, y2, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (z)", 0.5, z2, Delta);
  // roll and pitch are currently unused in stage ( returns set to 0 )
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (roll)", 0, roll2, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (pitch)", 0, pitch2, Delta);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("pose (yaw)", M_PI / 4, yaw2, Delta);
  CPPUNIT_ASSERT(time2 > time);

  // Return the robot to its starting point
  CPPUNIT_ASSERT(playerc_simulation_set_pose3d(simProxy, "r1", x, y, z, roll, pitch, yaw) == 0);
}

void Simulation::testProperties()
{
  int r0Agg = 5;
  int r1Agg = 1;
  int r1Pow = 125;

  // Set some properties
  CPPUNIT_ASSERT(
      playerc_simulation_set_property(simProxy, "r0", "aggression", &r0Agg, sizeof(void *)) == 0);
  CPPUNIT_ASSERT(
      playerc_simulation_set_property(simProxy, "r1", "aggression", &r1Agg, sizeof(void *)) == 0);
  CPPUNIT_ASSERT(playerc_simulation_set_property(simProxy, "r1", "power", &r1Pow, sizeof(void *))
                 == 0);

  // Get the properties back
  //	int r0Agg2, r1Agg2, r1Pow2;
  //	CPPUNIT_ASSERT( playerc_simulation_get_property( simProxy, "r0", "aggression", &r0Agg2,
  // sizeof(void*) ) == 0 );
  //	CPPUNIT_ASSERT( playerc_simulation_get_property( simProxy, "r1", "aggression", &r1Agg2,
  // sizeof(void*) ) == 0 );
  //	CPPUNIT_ASSERT( playerc_simulation_get_property( simProxy, "r1", "power", &r1Pow2,
  // sizeof(void*) ) == 0 );

  // Make sure they're the same
  //	CPPUNIT_ASSERT_EQUAL_MESSAGE( "r0Agg", r0Agg, r0Agg2 );
  //	CPPUNIT_ASSERT_EQUAL_MESSAGE( "r1Agg", r1Agg, r1Agg2 );
  //	CPPUNIT_ASSERT_EQUAL_MESSAGE( "r1Pow", r1Pow, r1Pow2 );
}
