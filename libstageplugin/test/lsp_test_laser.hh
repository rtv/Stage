#ifndef _LSP_LASER_TEST_H_
#define _LSP_LASER_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <libplayerc/playerc.h>

#include "lsp_test_proxy.hh"

namespace lspTest {
class Laser : public Proxy {
  CPPUNIT_TEST_SUITE(Laser);
  CPPUNIT_TEST(testConfig);
  CPPUNIT_TEST(testGeom);
  CPPUNIT_TEST(testData);
  CPPUNIT_TEST_SUITE_END();

protected:
  playerc_laser_t *laserProxy;

  void testConfig();
  void testGeom();
  void testData();

  static const int Samples;

public:
  void setUp();
  void tearDown();
};
};

CPPUNIT_TEST_SUITE_REGISTRATION(lspTest::Laser);

#endif
