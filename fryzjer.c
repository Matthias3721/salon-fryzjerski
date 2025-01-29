#include "salon.h"

void fryzjer(int id) {
    struct sembuf klient_czeka = { SEM_KLIENCI, -1, 0 };
    struct sembuf blokada_kolejki = { SEM_MUTEX_KOLEJKA, -1, 0 };
    struct sembuf odblokowanie_kolejki = { SEM_MUTEX_KOLEJKA, 1, 0 };
    struct sembuf zwolnienie_fotela = { SEM_FOTELE, 1, 0 };
    struct sembuf blokada_kasy = { SEM_KASA, -1, 0 };
    struct sembuf odblokowanie_kasy = { SEM_KASA, 1, 0 };

    while (salon_dane->salon_otwarty || salon_dane->klienci_w_poczekalni > 0) {
        semop(semid, &klient_czeka, 1);  // Czeka na klienta

        if (!salon_dane->salon_otwarty && salon_dane->klienci_w_poczekalni == 0) {
            break;  // Konczy pracę po zamknięciu salonu
        }

        semop(semid, &blokada_kolejki, 1);  // Blokuje dostęp do poczekalni
        salon_dane->klienci_w_poczekalni--;
        semop(semid, &odblokowanie_kolejki, 1);  // Odblokowuje dostęp

        printf("Fryzjer %d obsługuje klienta. Klienci w poczekalni: %d\n", id, salon_dane->klienci_w_poczekalni);
        sleep(2);  // Strzyżenie

        semop(semid, &blokada_kasy, 1);  // Blokada kasy
        salon_dane->kasa += 20;
        printf("Fryzjer %d: Klient zapłacił. Stan kasy: %d zł\n", id, salon_dane->kasa);
        semop(semid, &odblokowanie_kasy, 1);  // Odblokowanie kasy

        semop(semid, &zwolnienie_fotela, 1);  // Zwolnienie fotela
    }

    printf("Fryzjer %d kończy pracę.\n", id);
    exit(0);
}
