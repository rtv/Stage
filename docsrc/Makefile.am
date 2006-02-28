

# add these subdirs to the distribution tarball
EXTRA_DIST = \
README \
stage.txt \
header.html \
stage.dox \
stage_button.png 

DOCDIR = $(prefix)/share/doc

# build docs manually with 'make doc'
doc:
	doxygen $(srcdir)/stage.dox
	cp stage_button.png stage

# install the latest docs
doc-install: doc 
	install -d stage $(DOCDIR)
	cp -r -p -v stage $(DOCDIR)/stage

# zap the built docs
clean:
	rm -rf stage

# copy the latest docs to the web server
upload: 
	./upload.sh stage-cvs stage
