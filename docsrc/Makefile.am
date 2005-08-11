
# add these subdirs to the distribution tarball
EXTRA_DIST = stage stage_user


# make docs before building a distro
dist-hook: doc

# developers must build docs manually with 'make doc'
doc:
	doxygen stage.dox
	./build.sh
	doxygen stage_user.dox

# install the most recent docs built
install-data-hook:
	cp -r -p -v stage $(prefix)/doc
	cp stage_button.png stage
	cp stage_button.png stage_user

clean:
	rm -rf stage


# TODO
# upload:
# copy files to web server	