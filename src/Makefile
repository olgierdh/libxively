include ../Makefile.include

CREATE_DIRS := $(XI_BINDIR) $(XI_OBJDIR)

XI_SRCDIRS = libxively examples tests

SUB_BINDIRS = $(addprefix $(XI_BINDIR)/,$(XI_SRCDIRS))

#SUB_SRCDIRS := $(addprefix $(CURDIR)/,$(addsuffix "",$(XI_SRCDIRS)))
SUB_SRCDIRS := $(addprefix $(CURDIR)/,$(XI_SRCDIRS))

export XI_BINDIR
export XI_OBJDIR

# RULES:

.PHONY: $(SUB_SRCDIRS) libxively examples test clean

all: $(SUB_BINDIRS)

$(SUB_BINDIRS): | $(CREATE_DIRS)

$(SUB_BINDIRS): $(SUB_SRCDIRS)

$(CREATE_DIRS):
	mkdir -p $@

$(SUB_SRCDIRS):
	$(MAKE) -C $@ && cd ..

libxively:
	$(MAKE) -C $@

examples:
	$(MAKE) -C $@

test:
	$(MAKE) -C $@

clean:
	$(RM) -rf $(XI_BINDIR)
	$(RM) -rf $(XI_OBJDIR)
