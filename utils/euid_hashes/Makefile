# Definisce il nome del modulo finale
MODULE_NAME := euid_module

# Indica a Kbuild di generare un modulo loadable
obj-m := $(MODULE_NAME).o

# Specifica i file oggetto che compongono il modulo
$(MODULE_NAME)-y := euid_hash.o main.o

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
		rm /tmp/$(MODULE_NAME)_save.ko; \
	fi

# Target per la pulizia dei file generati
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

check:
	
