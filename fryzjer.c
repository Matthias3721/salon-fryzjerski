#include "salon.h"

void *fryzjer(void *arg) {
    int id = (long)arg;

    while (salon_otwarty || klienci_w_poczekalni > 0) {
        sem_wait(&sem_klienci);

        if (!salon_otwarty && klienci_w_poczekalni == 0) {
            break; // Zakończenie pracy po zamknięciu salonu
        }

        pthread_mutex_lock(&mutex_kolejka);
        klienci_w_poczekalni--;
        pthread_mutex_unlock(&mutex_kolejka);

        printf("Fryzjer %d obsługuje klienta. Klienci w poczekalni: %d\n", id, klienci_w_poczekalni);
        sleep(2);

        pthread_mutex_lock(&mutex_kasa);
        kasa += 20;
        printf("Fryzjer %d: Klient zapłacił. Stan kasy: %d zł\n", id, kasa);
        printf("Aktualny stan: klienci: %d, kasa: %d zł\n", klienci_w_poczekalni, kasa);
        pthread_mutex_unlock(&mutex_kasa);

        printf("Fryzjer %d skończył strzyżenie.\n", id);
        sem_post(&sem_fotele);
    }

    printf("Fryzjer %d kończy pracę.\n", id);
    return NULL;
}
