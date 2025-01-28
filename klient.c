#include "salon.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

void *klient(void *arg) {
    int id = (long)arg;

    pthread_mutex_lock(&mutex_kolejka);
    if (klienci_w_poczekalni < LICZBA_FOTELI) {
        klienci_w_poczekalni++;
        printf("Klient %d przyszedł do salonu. Miejsca w poczekalni: %d\n", id, klienci_w_poczekalni);
        printf("Aktualny stan: klienci: %d, kasa: %d zł\n", klienci_w_poczekalni, kasa);
        pthread_mutex_unlock(&mutex_kolejka);

        sem_post(&sem_klienci);  // Powiadamiamy fryzjera

        sem_wait(&sem_fotele);  // Klient czeka na fotel
        printf("Klient %d siada na fotelu.\n", id);
        sleep(1);  // Klient czeka na zakończenie strzyżenia

        printf("Klient %d wychodzi.\n", id);
    } else {
        pthread_mutex_unlock(&mutex_kolejka);
        printf("Klient %d odchodzi - brak miejsc w poczekalni.\n", id);
    }

    return NULL;
}
