SUBDIRS = replace src worlds docsrc examples

# create the pkg-config entries
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = stage.pc

clean-local:
	rm -f config.cache

LIBTOOL_DEPS = @LIBTOOL_DEPS@

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
