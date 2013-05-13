XI_OBJDIR ?= $(CURDIR)
include ../../Makefile.include

XI_COMM_LAYER ?= posix

XI_LAYER_DIRS := comm_layers/$(XI_COMM_LAYER)

XI_LAYERS_CFLAGS := -I./ $(foreach layerdir,$(XI_LAYER_DIRS),-I./$(layerdir))

XI_SOURCES = $(wildcard *.c) \
  $(wildcard comm_layers/$(XI_COMM_LAYER)/*.c)
XI_HEADERS = $(wildcard *.h) \
  $(wildcard comm_layers/$(XI_COMM_LAYER)/*.h)

XI_OBJECTS = $(XI_SOURCES:.c=.o)

XI_OBJS = $(addprefix $(XI_OBJDIR)/,$(XI_OBJECTS))

all: $(XI)

objs: $(XI_OBJS)

## The layers are in sub-directories, hence their objects. We could do something like this:
##	XI_OBJECTS = $(foreach obj,$(XI_SOURCES:.c=.o),$(foreach ldir,$(XI_LAYER_DIRS),$(subst $(ldir)/,,$(obj))))
## but that's pretty nasty, so we just make the sub-directories for those objects. Another
## option would be to compile each layer separately and then link to it's archive, but this
## would involve more makefiles. Making the directories doesn't cost anything really.
$(XI_OBJDIR)/%.o : %.c
	-mkdir -p $(dir $@)
	$(CC) $(XI_CFLAGS) $(XI_LAYERS_CFLAGS) $(CFLAGS) $< -o $@

$(XI): $(XI_OBJS)
	$(AR) $(XI_ARFLAGS) $(XI) $(XI_OBJS)

clean:
	$(RM) $(XI_OBJS) $(XI)
