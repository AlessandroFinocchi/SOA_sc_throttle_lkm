# Definisce il nome del modulo finale
MODULE_NAME := euid_module

# Indica a Kbuild di generare un modulo loadable
obj-m := $(MODULE_NAME).o

# Trova automaticamente tutti i file sorgente .c nelle directory specificate tranne i file main.c
MODULE_SRCS := main.c euid_hash.c

# Converte la lista di sorgenti .c in una lista di oggetti .o
$(MODULE_NAME)-objs := $(MODULE_SRCS:.c=.o)

# Aggiunge le directory degli header al percorso di ricerca del compilatore
INCLUDE_DIRS := $(sort $(dir $(wildcard ../../include/utils/)))
ccflags-y += $(addprefix -I,$(INCLUDE_DIRS))

# Percorso all'albero dei sorgenti o agli header del kernel in esecuzione
KDIR := /lib/modules/$(shell uname -r)/build

# Directory corrente
PWD := $(shell pwd)

# Target predefinito per la compilazione
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

	@if [ -f $(MODULE_NAME).ko ]; then \
		mv $(MODULE_NAME).ko /tmp/$(MODULE_NAME)_save.ko; \
	fi

	$(MAKE) -C $(KDIR) M=$(PWD) clean

	@if [ -f /tmp/$(MODULE_NAME)_save.ko ]; then \
		mv /tmp/$(MODULE_NAME)_save.ko $(MODULE_NAME).ko; \
	fi

# Target per la pulizia dei file generati
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean