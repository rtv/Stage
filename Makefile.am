SUBDIRS = rtk2 src worlds tools 

DOCSRC = $(top_srcdir)/docsrc
DOCDIST = $(distdir)/doc 
DOCINST = $(DESTDIR)$(prefix)/doc

stage_manual:
	cd $(DOCSRC) && make 

dist-hook: stage_manual
	mkdir -p $(DOCDIST)
	cp -p $(DOCSRC)/stage.ps $(DOCDIST)
	cp -p $(DOCSRC)/stage.pdf $(DOCDIST)

install-data-hook: stage_manual
	mkdir -p $(DOCINST)
	cp -p $(DOCSRC)/stage.ps $(DOCINST)
	cp -p $(DOCSRC)/stage.pdf $(DOCINST)

uninstall-local:
	rm -f $(DOCINST)/*

