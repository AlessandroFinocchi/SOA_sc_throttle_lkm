#!/bin/bash

if [ "$#" -lt 2 ]; then
    echo "Errore. Sintassi formale: $0 <nome_modulo> <ins|rm|ls>"
    exit 1
fi

MODULE=$1
ACTION=$2

# 2. Struttura di controllo principale
case "$ACTION" in
    "ins")
        echo "[+] Inserimento del modulo nel kernel: $MODULE"
        sudo insmod "$MODULE.ko"

        if [ $? -eq 0 ]; then
            echo "Inserimento completato con successo."
        else
            echo "Errore durante l'inserimento. Verificare tramite 'dmesg | tail'."
        fi
        ;;

    "rm")
        echo "[-] Rimozione del modulo dal kernel: $MODULE"
        sudo rmmod "$MODULE"
        
        if [ $? -eq 0 ]; then
            echo "Rimozione completata con successo."
        else
            echo "Errore durante la rimozione. Verificare i riferimenti attivi."
        fi
        ;;

    "ls")
        echo "[*] Ispezione dello stato del modulo:"
        lsmod | grep "$MODULE"
        
        if [ $? -ne 0 ]; then
            echo "Il modulo '$MODULE' non è attualmente allocato in memoria."
        fi
        ;;

    *)
        # 3. Gestione del default e codici di errore
        echo "Errore di sintassi: L'operatore '$ACTION' non è supportato."
        echo "Operatori ammessi: 'ins' (inserimento), 'rm' (rimozione), 'ls' (ispezione)."
        exit 1
        ;;
esac