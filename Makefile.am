SUBDIRS = replace src worlds docsrc

DOCSRC = $(top_srcdir)/docsrc
DOCDIST = $(distdir)/docsrc
DOCINST = $(DESTDIR)$(prefix)/doc

# create the pkg-config entries
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = stage.pc

#dist-hook:
#	cd $(DOCSRC) && make 
#	mkdir -p $(DOCDIST)
#	cp -r -p $(DOCSRC)/stage_user $(DOCDIST)

#	cp -r -p $(DOCSRC)/stage_reference $(DOCDIST)

#install-data-hook:
#	mkdir -p $(DOCINST)

#	cd $(DOCSRC) && make 
#	cp -r -p $(DOCSRC)/stage_reference $(DOCINST)
#	cp -r -p $(DOCSRC)/stage_user $(DOCINST)

#uninstall-local:
#	rm -f $(DOCINST)/*

clean-local:
	rm -f config.cache

LIBTOOL_DEPS = @LIBTOOL_DEPS@

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
