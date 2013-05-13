REV := $(shell git rev-parse --short HEAD)

all: docs
	$(MAKE) -C src
	./src/bin/libxively_unit_test_suite
docs:
	doxygen
clean:
	-rm -rf doc/html
	$(MAKE) -C src $@

# This provides a very easy way of importing the code base into mbed.org IDE.
# We create a zip archive with shortened revision hash in the filename
# To import the library into a project, click Import button and then select
# upload tab and navigate to the location of this zip archive.
# When the IDE unpacks the library, it magically names it by basename of the
# zip archive, so revision hash appears in the lib's name.
mbed_zipfile: mbed-libxively-$(REV)
mbed-libxively-$(REV):
	zip "$@" \
	   src/libxively/*.[ch] \
	   src/libxively/comm_layers/mbed/*

update_docs_branch:
	-rm -rf doc/html
	doxygen && cd doc/html \
		&& git init \
		&& git remote add github git@github.com:xively/libxively \
		&& git add . \
		&& git commit -m "[docs] Regerated documentation for $(REV)" \
		&& git push github master:gh-pages -f
