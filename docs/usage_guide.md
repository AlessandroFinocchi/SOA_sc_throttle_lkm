# Manuale d'uso


## Gestione del modulo
Il modulo va gestito dentro la cartella root del progetto.

### Compilazione
E' possibile compilare il modulo con 3 configurazioni:
1. Thread interrompibili con politica di risveglio decisa dallo scheduler.
    ```
    make
    ```
2. Thread non interrompibili con politica di risveglio decisa dallo scheduler.
    ```
    make KCFLAGS="-DWEQ_UNINT"
    ```
3. Thread non interrompibili con politica di risveglio FIFO
    ```
    make KCFLAGS="-DPRIO_FIFO"
    ```
Inoltre è possibile aggiungere il flag `DEBUG` per rendere più verboso il modulo a runtime, aggiungendolo ai `KCFLAGS`
```
make KCFLAGS="-DPRIO_FIFO -DDEBUG"
```

### Pulizia directory
```
make clean
```

### Inserimento
```
make ins
```

### Rimozione
```
make rm
```


## Test/User suite

### Device
1. Dalla cartella root del progetto entrare nella sezione dei test per il character device
    ```sh
    cd tests/devices/
    ```
2. Compilare tutta la test suite
    ```sh
    make
    ```
3. Eseguire il test
    ```sh
    ./<test_program_name>
    ```
Per ulteriori dettagli sui test, consultare la [Documentazione sulla test suite](tests.md).


### Probes
1. Dalla cartella root del progetto entrare nella sezione dei test per il le kernel probes
    ```sh
    cd tests/probes/
    ```
2. Compilare tutta la test suite
    ```sh
    make
    ```
3. Eseguire il test
    ```sh
    ./<test_program_name>
    ```
Per ulteriori dettagli sui test, consultare la [Documentazione sulla test suite](tests.md).

