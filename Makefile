# Nome del modulo finale condiviso da entrambi i passi
MODULE_NAME := throttler_module

# ==============================================================================
# KBUILD PASS (Passo 2: eseguito all'interno della directory del Kernel)
# ==============================================================================
ifneq ($(KERNELRELEASE),)

# Indica a Kbuild di generare un modulo loadable
obj-m := $(MODULE_NAME).o

# 1. Ancoraggio Assoluto: utilizziamo $(src) per forzare il wildcard a cercare
#    i file sorgente all'interno della directory reale del modulo esterno.
ABS_SRCS := $(filter-out %main.c, \
               $(wildcard $(src)/devices/*.c) \
               $(wildcard $(src)/probes/*.c) \
               $(wildcard $(src)/services/*.c) \
               $(wildcard $(src)/utils/ds/*.c))

# 2. Trasformazione Relativa: Kbuild esige che i target in *-objs siano relativi 
#    alla root del modulo. Mappiamo i path assoluti in relativi e aggiungiamo sctrt.o.
REL_OBJS := $(patsubst $(src)/%.c, %.o, $(ABS_SRCS)) sctrt.o

# Assegnamento degli oggetti finali al modulo
$(MODULE_NAME)-objs := $(REL_OBJS)

# 3. Risoluzione degli Header: cerchiamo le directory di include locali tramite $(src)
#    e passiamo i path assoluti risultanti al compilatore tramite -I.
INCLUDE_DIRS := $(sort $(dir $(wildcard $(src)/include/ $(src)/include/*/)))
ccflags-y += $(addprefix -I, $(INCLUDE_DIRS))

# Flag di compilazione
# ccflags-y += -DWEQ_UNINT
ccflags-y += -DDEBUG

# ==============================================================================
# STANDARD MAKE PASS (Passo 1: eseguito dall'utente da riga di comando)
# ==============================================================================
else

# Percorso all'albero dei sorgenti o agli header del kernel in esecuzione
KDIR := /lib/modules/$(shell uname -r)/build

# Directory corrente
PWD := $(shell pwd)

# Compilazione del modulo kernel con logica di salvataggio del binario
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

	@if [ -f $(MODULE_NAME).ko ]; then \
		mv $(MODULE_NAME).ko /tmp/$(MODULE_NAME)_save.ko; \
	fi

	$(MAKE) -C $(KDIR) M=$(PWD) clean

	@if [ -f /tmp/$(MODULE_NAME)_save.ko ]; then \
		mv /tmp/$(MODULE_NAME)_save.ko $(MODULE_NAME).ko; \
	fi

# Pulizia file generati
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

ins:
	sudo ./mod.sh throttler_module ins

ls:
	sudo ./mod.sh throttler_module ls

rm:
	sudo ./mod.sh throttler_module rm

endif