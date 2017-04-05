#ifndef _LSP_SPEECH_TEST_H_
#define _LSP_SPEECH_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <libplayerc/playerc.h>

#include "lsp_test_proxy.hh"

namespace lspTest {
class Speech : public Proxy {
  CPPUNIT_TEST_SUB_SUITE(Speech, Proxy);
  CPPUNIT_TEST(testSay1);
  CPPUNIT_TEST_SUITE_END();

protected:
  playerc_speech_t *speechProxy;

  void testSay1();

public:
  void setUp();
  void tearDown();
};
};

CPPUNIT_TEST_SUITE_REGISTRATION(lspTest::Speech);

#endif
