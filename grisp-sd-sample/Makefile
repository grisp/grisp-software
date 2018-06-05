RTEMS_ROOT ?= $(PWD)/../rtems-install/rtems/5
RTEMS_BSP ?= atsamv

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

CFLAGS += -O0
LDFLAGS += -qnolinkcmds -T linkcmds.sdram

APP = $(BUILDDIR)/sd-sample
APP_PIECES = $(wildcard *.c)
APP_OBJS = $(APP_PIECES:%.c=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%.c=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(APP).exe

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -lgrisp -lbsd -lm -o $@

$(APP).bin: $(APP).exe
	$(OBJCOPY) -O binary $^ $@

bin: all $(APP).bin

clean:
	rm -rf $(BUILDDIR)

debugoutput:
	echo $(PATH)

-include $(APP_DEPS)
