SUBDIRS = replace src worlds tools 
#config

DOCSRC = $(top_srcdir)/docsrc
DOCDIST = $(distdir)/doc 
DOCINST = $(DESTDIR)$(prefix)/doc

# create the pkg-config entries
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = stage.pc stageplugin.pc

dist-hook:
	cd $(DOCSRC) && make 
	mkdir -p $(DOCDIST)
	cp -r -p $(DOCSRC)/html $(DOCDIST)
	cp -p $(DOCSRC)/*.txt $(DOCDIST)
	cp -p stage.xpm $(distdir)

install-data-hook:
	mkdir -p $(DOCINST)
	mkdir -p $(DOCINST)/stage
	cp -r -p $(top_srcdir)/docsrc/stage_user $(DOCINST)
	cp -r -p $(top_srcdir)/docsrc/stage_developer $(DOCINST)


uninstall-local:
	rm -f $(DOCINST)/*

clean-local:
	rm -f config.cache

LIBTOOL_DEPS = @LIBTOOL_DEPS@

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
