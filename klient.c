#include "salon.h"

void klient(int id) {
    struct sembuf blokada_kolejki = { SEM_MUTEX_KOLEJKA, -1, 0 };
    struct sembuf odblokowanie_kolejki = { SEM_MUTEX_KOLEJKA, 1, 0 };
    struct sembuf wysylanie_klienta = { SEM_KLIENCI, 1, 0 };

    semop(semid, &blokada_kolejki, 1);  // Blokada dostępu do poczekalni

    if (salon_dane->klienci_w_poczekalni < LICZBA_FOTELI) {
        salon_dane->klienci_w_poczekalni++;
        printf("Klient %d przyszedł do salonu. Klienci w poczekalni: %d\n", id, salon_dane->klienci_w_poczekalni);
        
        semop(semid, &odblokowanie_kolejki, 1);  // Odblokowanie dostępu do poczekalni
        semop(semid, &wysylanie_klienta, 1);    // Powiadomienie fryzjera

        struct sembuf czeka_na_fotel = { SEM_FOTELE, -1, 0 };
        semop(semid, &czeka_na_fotel, 1);  // Czeka na fotel

        printf("Klient %d siada na fotelu.\n", id);
        sleep(1);  // Strzyzenie

        printf("Klient %d wychodzi.\n", id);
    } else {
        semop(semid, &odblokowanie_kolejki, 1);  // Odblokowanie poczekalni
        printf("Klient %d odchodzi - brak miejsc w poczekalni.\n", id);
    }

    exit(0);
}
