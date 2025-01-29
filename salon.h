#ifndef SALON_H
#define SALON_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>

#define LICZBA_FRYZJERÓW 100
#define LICZBA_KLIENTÓW 2000
#define LICZBA_FOTELI 50

extern sem_t sem_fotele;
extern sem_t sem_klienci;
extern pthread_mutex_t mutex_kasa;
extern pthread_mutex_t mutex_kolejka;
extern int kasa;
extern int klienci_w_poczekalni;
extern volatile sig_atomic_t salon_otwarty;

void *fryzjer(void *arg);
void *klient(void *arg);

#endif
