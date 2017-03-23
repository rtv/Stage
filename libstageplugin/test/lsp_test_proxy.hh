#ifndef _LSP_PROXY_TEST_H_
#define _LSP_PROXY_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <libplayerc/playerc.h>

namespace lspTest {
class Proxy : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Proxy);
  CPPUNIT_TEST_SUITE_END_ABSTRACT();

protected:
  playerc_client_t *client;

  void connect();
  void disconnect();

  static const double Delta;
};
};

#endif
