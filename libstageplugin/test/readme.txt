Building
--------
CPPUNIT is required.  Build by enabling the BUILD_LSPTEST option either with a cmake command line arg or through ccmake on the root of the stage source.

Execution
---------
Run player on lsp_test.cfg, included in the worlds section of the stage source.

Execute lsp_test on the same machine and the test suites will connect to localhost:6665 to perform testing.