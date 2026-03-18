# Makefile per compilare moduli di test diversi con obiettivi specifici

# Impostazioni di default
MODULE_NAME ?= euid_module
MODULE_SRCS ?= test_euid_hash.c ../../utils/ds/euid_hash.c

# Kbuild module definitions
obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(MODULE_SRCS:.c=.o)

# Include directory
INCLUDE_DIRS := $(sort $(dir $(wildcard ../../include/utils/)))
ccflags-y += $(addprefix -I,$(INCLUDE_DIRS))

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: all euid str clean _build
all: euid

euid:
	$(MAKE) MODULE_NAME=euid_module \
	MODULE_SRCS="test_euid_hash.c ../../utils/ds/euid_hash.c" \
	_build

str:
	$(MAKE) MODULE_NAME=str_module \
	MODULE_SRCS="test_str_hash.c ../../utils/ds/str_hash.c" \
	 _build

scb:
	$(MAKE) MODULE_NAME=scb_module \
	MODULE_SRCS="test_sc_bitmap.c ../../utils/ds/sc_bitmap.c" \
	 _build

_build:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	@if [ -f $(MODULE_NAME).ko ]; then \
		mv $(MODULE_NAME).ko /tmp/$(MODULE_NAME)_save.ko; \
	fi
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@if [ -f /tmp/$(MODULE_NAME)_save.ko ]; then \
		mv /tmp/$(MODULE_NAME)_save.ko $(MODULE_NAME).ko; \
	fi

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean