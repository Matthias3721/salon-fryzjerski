#include "salon.h"

void *kierownik(void *arg) {
    sleep(20); // Symulacja czasu pracy salonu

    printf("Kierownik zamyka salon!\n");
    return NULL;
}
