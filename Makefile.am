SUBDIRS = src worlds tools 

DOCSRC = $(top_srcdir)/docsrc
DOCDIST = $(distdir)/doc 
DOCINST = $(DESTDIR)$(prefix)/share/doc

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

# need to figure out a safer way to do this
#uninstall-local:
#	rm -f $(DOCINST)/*

