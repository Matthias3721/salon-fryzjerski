#ifndef SALON_H
#define SALON_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define LICZBA_FRYZJERÓW 50
#define LICZBA_KLIENTÓW 300
#define LICZBA_FOTELI 200

extern sem_t sem_fotele;
extern sem_t sem_klienci;
extern pthread_mutex_t mutex_kasa;
extern pthread_mutex_t mutex_kolejka;
extern int kasa;
extern int klienci_w_poczekalni;
extern int salon_otwarty;

void *fryzjer(void *arg);
void *klient(void *arg);

#endif
