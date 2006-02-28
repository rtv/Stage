

# add these subdirs to the distribution tarball
EXTRA_DIST = \
README \
stage.txt \
header_top.src \
header_bottom.src \
stage.dox \
stage_button.png 


DOCDIR = $(prefix)/share/doc


# build docs manually with 'make doc'
doc:
	$(srcdir)/build.sh
	doxygen $(srcdir)/stage.dox
	cp stage_button.png stage

# build and install the docs
doc-install: doc 
	cp -r -p -v stage $(DOCDIR) 


# zap the built docs
clean:
	rm -rf stage

# build and copy the docs to the web server
upload: doc
	./upload.sh stage-cvs stage
