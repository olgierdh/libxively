LIBXIVELY := $(shell git rev-parse --show-toplevel)

include $(LIBXIVELY)/Makefile.include

ifeq ($(XI_NOB_ENABLED),true)
	XI_CFLAGS += -DXI_NOB_ENABLED
endif

XI_EXAMPLE_SOURCES = $(wildcard *.c)
XI_CFLAGS += -I../libxively/
XI_EXAMPLES = $(addprefix $(XI_BINDIR)/,$(XI_EXAMPLE_SOURCES:.c=))

all: $(XI_EXAMPLES)

$(XI_BINDIR)/%: %.c $(XI)
	@-mkdir -p $(dir $@)
	$(CC) $(XI_CFLAGS) $^ -o $@

$(XI):
	$(MAKE) -C .. libxively

include $(LIBXIVELY)/Makefile.rules
