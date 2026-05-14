# Manuale d'uso


## Gestione del modulo
Il modulo va gestito dentro la cartella root del progetto.

### Compilazione
```
make all
```

### Inserimento
```
make ins
```

### Rimozione
```
make rm
```


## Test suite

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

