
# add these subdirs to the distribution tarball
EXTRA_DIST = stage


# make docs before building a distro
dist-hook: doc

# developers must build docs manually with 'make doc'
doc:
	doxygen stage.dox

# install the most recent docs built
install-data-hook:
	cp -r -p -v stage $(prefix)/doc
