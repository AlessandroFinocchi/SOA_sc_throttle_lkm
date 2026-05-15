# https://elixir.bootlin.com/linux/v6.12.73/source/Documentation/kbuild/modules.rst
# Nome del modulo
MODULE_NAME := throttler_module

# ==============================================================================
# KBUILD PASS (Passo 2: eseguito all'interno della directory del Kernel)
# ==============================================================================
ifneq ($(KERNELRELEASE),)

# Indica a Kbuild di generare un modulo loadable
obj-m := $(MODULE_NAME).o

# 1. Path Assoluti: trova automaticamente tutti i file sorgente 
#    .c nelle directory specificate (tranne i file main.c)
ABS_SRCS := $(filter-out %main.c, \
               $(wildcard $(src)/devices/*.c) \
               $(wildcard $(src)/probes/*.c) \
               $(wildcard $(src)/services/*.c) \
               $(wildcard $(src)/utils/ds/*.c))

# 2. Trasformazione Relativa: Kbuild esige che i target in *-objs siano relativi
REL_OBJS := $(patsubst $(src)/%.c, %.o, $(ABS_SRCS)) sctrt.o

# Converte la lista di sorgenti .c in una lista di oggetti .o
$(MODULE_NAME)-objs := $(REL_OBJS)

# Aggiunge le directory degli header al percorso di ricerca del compilatore
INCLUDE_DIRS := $(sort $(dir $(wildcard $(src)/include/ $(src)/include/*/)))

# Flag di compilazione
ccflags-y += $(addprefix -I, $(INCLUDE_DIRS))
# ccflags-y += -DPRIO_FIFO # => WEQ_UNINT
# ccflags-y += -DWEQ_UNINT
# ccflags-y += -DDEBUG

# ==============================================================================
# STANDARD MAKE PASS (Passo 1: eseguito dall'utente da riga di comando)
# ==============================================================================
else

# Percorso all'albero dei sorgenti o agli header del kernel in esecuzione
KDIR := /lib/modules/$(shell uname -r)/build

# Directory corrente
PWD := $(shell pwd)

all:
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


ins:
	sudo ./mod.sh throttler_module ins

ls:
	sudo ./mod.sh throttler_module ls

rm:
	sudo ./mod.sh throttler_module rm

endif