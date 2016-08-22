RTEMS_ROOT = $(PWD)/../rtems-install/rtems-4.12
RTEMS_BSP ?= atsamv

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

CPPFLAGS += -Iinclude

LIB = $(BUILDDIR)/libgrisp.a
LIB_PIECES = $(wildcard src/*.c)
LIB_OBJS = $(LIB_PIECES:%.c=$(BUILDDIR)/%.o)
LIB_DEPS = $(LIB_PIECES:%.c=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(LIB)

install: all
	mkdir -p $(PROJECT_INCLUDE)/grisp
	install -m 644 $(LIB) $(PROJECT_LIB)
	install -m 644 include/grisp/*.h $(PROJECT_INCLUDE)/grisp

$(BUILDDIR):
	mkdir $(BUILDDIR)
	mkdir $(BUILDDIR)/src

$(LIB): $(LIB_OBJS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

clean:
	rm -rf $(BUILDDIR)

-include $(LIB_DEPS)
