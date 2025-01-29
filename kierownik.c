#include "salon.h"
#include "kierownik.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


void kierownik() {
    sleep(5);
    printf("Kierownik zamyka salon!\n");

    salon_dane->salon_otwarty = 0;

    for (int i = 0; i < LICZBA_FRYZJERÓW; i++) {
        semctl(sem_klienci, 0, SETVAL, 1);
    }

    printf("Ostateczny stan: klienci w poczekalni: %d, kasa: %d zł\n",
           salon_dane->klienci_w_poczekalni, salon_dane->kasa);
}

