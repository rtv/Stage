SUBDIRS = replace src worlds tools 
#config

DOCSRC = $(top_srcdir)/docsrc
DOCDIST = $(distdir)/doc 
DOCINST = $(DESTDIR)$(prefix)/doc

# create the pkg-config entries
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = stage.pc

dist-hook:
	cd $(DOCSRC) && make 
	mkdir -p $(DOCDIST)
	cp -p $(DOCSRC)/stage.ps $(DOCDIST)
	cp -p $(DOCSRC)/stage.pdf $(DOCDIST)
	cp -p stage.xpm $(distdir)

install-data-hook:
	mkdir -p $(DOCINST)
	cp -p $(top_srcdir)/doc/stage.ps $(DOCINST)
	cp -p $(top_srcdir)/doc/stage.pdf $(DOCINST)

uninstall-local:
	rm -f $(DOCINST)/*

clean-local:
	rm -f config.cache