

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
	./sourcedocs.sh > sourced.txt
	doxygen stage.dox
	cp stage_button.png stage
	rm -f sourced.txt

# install the latest docs
doc-install: doc 
	install -d stage $(DOCDIR)
	cp -r -p -v stage $(DOCDIR)/stage

# zap the built docs
clean:
	rm -rf stage sourced.txt

# copy the latest docs to the web server
upload: 
		./ghp-import -p stage
