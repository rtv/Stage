SUBDIRS = src worlds tools

dist-hook:
	mkdir -p $(top_distdir)/doc
	cd $(srcdir)/doc && make ps
	cp -p $(srcdir)/doc/stage.ps $(top_distdir)/doc/stage.ps

docdir = $(prefix)/doc
install-data-hook:
	mkdir -p $(DESTDIR)$(prefix)/doc/
	cp $(top_srcdir)/doc/* $(DESTDIR)$(prefix)/doc/

uninstall-local:
	rm -f $(DESTDIR)$(docdir)/*

