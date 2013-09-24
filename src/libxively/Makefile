LIBXIVELY := $(shell git rev-parse --show-toplevel)
include $(LIBXIVELY)/Makefile.include

XI_LAYER_DIRS := comm_layers/$(XI_COMM_LAYER)

XI_EXTRA_CFLAGS += -I./ \
	$(foreach layerdir,$(XI_LAYER_DIRS),-I./$(layerdir))

XI_SOURCES = $(wildcard *.c) \
	$(wildcard comm_layers/$(XI_COMM_LAYER)/*.c)

all: $(XI)

objs: $(XI_OBJS)
deps: $(XI_DEPS)

$(XI): $(XI_OBJS)
	$(AR) $(XI_ARFLAGS) $(XI) $(XI_OBJS)

include $(LIBXIVELY)/Makefile.rules

ifneq ($(MAKECMDGOALS),clean)
  -include $(XI_DEPS)
endif
