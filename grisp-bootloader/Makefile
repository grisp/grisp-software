RTEMS_ROOT ?= $(PWD)/../rtems-install/rtems-4.12
RTEMS_BSP ?= atsamv

include $(RTEMS_ROOT)/make/custom/$(RTEMS_BSP).mk

GRISP_BL_VERSION := $(shell git describe --abbrev=4 --dirty --always --tags)
GRISP_BL_BUILD_DATE := $(shell LC_LOCALE=en_US date -u)
#CFLAGS += -O0
CFLAGS += -DGRISP_BL_VERSION="\"$(GRISP_BL_VERSION)\""
CFLAGS += -DGRISP_BL_BUILD_DATE="\"$(GRISP_BL_BUILD_DATE)\""
LDFLAGS += -qnolinkcmds -T linkcmds.bootloader
#LDFLAGS += -qnolinkcmds -T linkcmds.sdram

APP = $(BUILDDIR)/bootloader
APP_PIECES = $(wildcard *.c)
APP_OBJS = $(APP_PIECES:%.c=$(BUILDDIR)/%.o)
APP_DEPS = $(APP_PIECES:%.c=$(BUILDDIR)/%.d)

all: $(BUILDDIR) $(APP).exe

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(APP).exe: $(APP_OBJS)
	$(CCLINK) $^ -lgrisp -linih -lbsd -o $@

clean:
	rm -rf $(BUILDDIR)

-include $(APP_DEPS)
