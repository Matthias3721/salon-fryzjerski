#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_FOTELE 3  // Ilość dostępnych foteli w salonie

pthread_mutex_t mutex_fotele = PTHREAD_MUTEX_INITIALIZER;
int wolne_fotele = MAX_FOTELE;

void *klient(void *id) {
    int klient_id = (intptr_t) id;

    printf("Klient %d przyszedl do salonu.\n", klient_id);

    pthread_mutex_lock(&mutex_fotele);
    if (wolne_fotele > 0) {
        wolne_fotele--;
        printf("Klient %d zajmuje fotel. Wolne fotele: %d\n", klient_id, wolne_fotele);
        pthread_mutex_unlock(&mutex_fotele);

        sleep(rand() % 3 + 1); // Czas strzyżenia

        pthread_mutex_lock(&mutex_fotele);
        wolne_fotele++;
        printf("Klient %d skonczyl strzyzenie i wychodzi. Wolne fotele: %d\n", klient_id, wolne_fotele);
        pthread_mutex_unlock(&mutex_fotele);
    } else {
        printf("Klient %d odchodzi - brak miejsc w poczekalni.\n", klient_id);
        pthread_mutex_unlock(&mutex_fotele);
    }

    return NULL;
}
