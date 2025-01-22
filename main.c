#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_KLIENCI 10  // Maksymalna liczba klientów

void *klient(void *id);  // Prototyp funkcji klienta

int main() {
    pthread_t klienci[MAX_KLIENCI];

    printf("Start programu: Salon fryzjerski\n");

    // Tworzenie klientów jako wątków
    for (int i = 0; i < MAX_KLIENCI; i++) {
        pthread_create(&klienci[i], NULL, klient, (void *)(intptr_t)i);
        usleep(100000); // Małe opóźnienie między klientami
    }

    // Oczekiwanie na zakończenie klientów
    for (int i = 0; i < MAX_KLIENCI; i++) {
        pthread_join(klienci[i], NULL);
    }

    printf("Wszyscy klienci obsluzeni. Zamykanie salonu.\n");

    return 0;
}
