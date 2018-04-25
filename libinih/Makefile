RTEMS_ROOT ?= $(PWD)/../rtems-install/rtems/5
RTEMS_BSP ?= atsamv

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

# inih uses a slightly odd cast. The way it is used, it should be no problem.
# Therefore make an exception for this error.
CFLAGS += -Wno-error=cast-qual

LIB = $(BUILDDIR)/libinih.a
LIB_PIECES = $(wildcard inih/*.c)
LIB_OBJS = $(LIB_PIECES:%.c=$(BUILDDIR)/%.o)
LIB_DEPS = $(LIB_PIECES:%.c=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(LIB)

install: all
	mkdir -p $(PROJECT_INCLUDE)/inih
	install -m 644 $(LIB) $(PROJECT_LIB)
	install -m 644 inih/*.h $(PROJECT_INCLUDE)/inih

$(BUILDDIR):
	mkdir $(BUILDDIR)
	mkdir $(BUILDDIR)/inih

$(LIB): $(LIB_OBJS)
	$(AR) rcu $@ $^
	$(RANLIB) $@

clean:
	rm -rf $(BUILDDIR)

-include $(LIB_DEPS)
