SUBDIRS = src worlds tools replace

DOCSRC = $(top_srcdir)/docsrc
DOCDIST = $(distdir)/doc 
DOCINST = $(DESTDIR)$(prefix)/doc

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

