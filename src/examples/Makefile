EXAMPLE_DIRS = \
	       datapoint_delete \
	       datapoint_delete_range \
	       datastream_create \
	       datastream_delete \
	       datastream_get \
	       datastream_update \
	       feed_get \
	       feed_update

all:
	for dir in $(EXAMPLE_DIRS); do ($(MAKE) -C $$dir) || exit 1; done
