#include "salon.h"
#include <stdio.h>
#include <stdlib.h>

// Definicje globalnych zmiennych
sem_t sem_fotele;
pthread_mutex_t mutex_kolejka;
int klienci_w_poczekalni = 0;

// Funkcja inicjalizująca salon
void inicjalizuj_salon() {
    sem_init(&sem_fotele, 0, LICZBA_FOTELI);
    pthread_mutex_init(&mutex_kolejka, NULL);
}

// Funkcja kończąca działanie salonu
void zakoncz_salon() {
    sem_destroy(&sem_fotele);
    pthread_mutex_destroy(&mutex_kolejka);
}
