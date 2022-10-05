# BDM-interface
Implementazione di un'interfaccia BDM con Raspberry Pi Pico
 
# Inizializzazione
## Active background mode
Una volta collegato il dispositivo all'alimentazione, il pin BKGD verrà tenuto al livello basso per 5 secondi, per dare modo all'MCU di entrare nella active background mode. 
## Sincronizzazione
Una volta trascorsi i 5 secondi, il dispositivo trasmetterà un comando di sincronizzazione, per recuperare la frequenza del dispositivo target. In particolare il pin BKGD verrà mandato basso per 128 colpi di clock alla frequenza di [SYNC_FREQ](config.h) e poi tornerà alto, aspettando che il target lo porti basso per 128 colpi di clock alla sua frequenza incognita, per poi tornare alto. Il dispositivo misura il tempo che trascorre tra il momento in cui il target porta il pin basso a quando lo riporta alto e calcola la frequenza del target. Ogni [SYNC_COUNT_THRESHOLD](config.h) comandi, verrà rigenerato un comando di sincronizzazione. 

# Dare comandi
Per dare un [comando](commands.h) è sufficiente digitare il codice del comando(ad esempio D5), seguito da:
- ?? o ???? per segnalare la lettura di 8 o 16 bit;
- XX o XXXX per segnalare la trasmissione di 8 o 16 bit(le X dovranno essere sostituite con il valore da trasmettere).
Ogni parola dovrà essere separata dall'altra dal carattere ':'.


