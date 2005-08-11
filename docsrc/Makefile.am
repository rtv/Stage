
DOCDIR = stage

# add these subdirs to the distribution tarball
EXTRA_DIST = $(DOCDIR)

# make docs before building a distro
dist-hook: doc

# developers must build docs manually with 'make doc'
doc:
	./build.sh
	doxygen stage.dox
	cp stage_button.png $(DOCDIR)

# install the most recent docs built
install-data-hook:
	cp -r -p -v $(DOCDIR) $(prefix)/doc


clean:
	rm -rf $(DOCDIR)

# copy the current documentation to the web server
upload: 
	./upload.sh stage-cvs $(DOCDIR)
