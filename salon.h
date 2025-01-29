#ifndef SALON_H
#define SALON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

#define LICZBA_FRYZJERÓW 100
#define LICZBA_KLIENTÓW 2000
#define LICZBA_FOTELI 50

// Struktura pamięci współdzielonej
typedef struct {
    int kasa;
    int klienci_w_poczekalni;
    int salon_otwarty;
} ShmSalon;

// Globalne zmienne dla pamięci współdzielonej i semaforów
extern int shm_id;
extern ShmSalon *salon_dane;
extern int semid;

// Indeksy semaforów
#define SEM_FOTELE 0
#define SEM_KLIENCI 1
#define SEM_KASA 2
#define SEM_MUTEX_KOLEJKA 3

// Funkcje
void setup_ipc();
void cleanup_ipc();
void sigint_handler(int signum);
void kierownik();
void klient();
void fryzjer();

#endif
