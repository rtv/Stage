SUBDIRS = replace src worlds tools

DOCSRC = $(top_srcdir)/docsrc
DOCDIST = $(distdir)/docsrc
DOCINST = $(DESTDIR)$(prefix)/doc

# create the pkg-config entries
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = stage.pc

#dist-hook:
#	cd $(DOCSRC) && make 
#	mkdir -p $(DOCDIST)
#	cp -r -p $(DOCSRC)/stage_developer $(DOCDIST)
#	cp -r -p $(DOCSRC)/stage_user $(DOCDIST)

#install-data-hook:
#	mkdir -p $(DOCINST)
#	mkdir -p $(DOCINST)/stage
#	cp -r -p $(DOCSRC)/stage_user $(DOCINST)
#	cp -r -p $(DOCSRC)/stage_developer $(DOCINST)

uninstall-local:
	rm -f $(DOCINST)/*

clean-local:
	rm -f config.cache

LIBTOOL_DEPS = @LIBTOOL_DEPS@

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
