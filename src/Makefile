LIBXIVELY := $(shell git rev-parse --show-toplevel)

include $(LIBXIVELY)/Makefile.include
include tests/Makefile.defaults

XI_COMM_LAYER ?= posix

export XI_BINDIR
export XI_OBJDIR

.PHONY: libxively tests clean


libxively:
	$(MAKE) -C $@

#examples:
#	$(MAKE) -C $@

tests: clean libxively
	$(MAKE) -C $@

include $(LIBXIVELY)/Makefile.rules
