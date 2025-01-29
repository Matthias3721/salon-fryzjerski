#include "salon.h"
#include <signal.h>

sem_t sem_fotele;
sem_t sem_klienci;
pthread_mutex_t mutex_kasa = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_kolejka = PTHREAD_MUTEX_INITIALIZER;

int kasa = 100; // Początkowa kwota w kasie
int klienci_w_poczekalni = 0;
volatile sig_atomic_t salon_otwarty = 1; // 'volatile' zapewnia poprawne działanie w sygnałach

pthread_t fryzjerzy[LICZBA_FRYZJERÓW];
pthread_t klienci[LICZBA_KLIENTÓW];
pthread_t manager;

void sigint_handler(int signum) {
    printf("\nOtrzymano SIGINT! Zamykanie salonu natychmiast...\n");
    salon_otwarty = 0;

    // Odblokowanie semaforów, aby zakończyć czekające wątki
    for (int i = 0; i < LICZBA_FRYZJERÓW; i++) {
        sem_post(&sem_klienci);
    }

    // Anulowanie wątków klientów i fryzjerów
    for (int i = 0; i < LICZBA_FRYZJERÓW; i++) {
        pthread_cancel(fryzjerzy[i]);
    }
    for (int i = 0; i < LICZBA_KLIENTÓW; i++) {
        pthread_cancel(klienci[i]);
    }

    pthread_cancel(manager); // Zakończenie wątku kierownika

    sem_destroy(&sem_fotele);
    sem_destroy(&sem_klienci);

    printf("\nOstateczny stan: klienci w poczekalni: %d, kasa: %d zł\n", klienci_w_poczekalni, kasa);
    printf("Salon fryzjerski zamknięty!\n");
    
    exit(0); // Natychmiastowe zakończenie programu
}

void *kierownik(void *arg) {
    sleep(5);  // Czas działania salonu
    printf("Kierownik zamyka salon!\n");

    salon_otwarty = 0;

    for (int i = 0; i < LICZBA_FRYZJERÓW; i++) {
        sem_post(&sem_klienci);
    }

    printf("Ostateczny stan: klienci w poczekalni: %d, kasa: %d zł\n", klienci_w_poczekalni, kasa);
    return NULL;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    sem_init(&sem_fotele, 0, LICZBA_FOTELI);
    sem_init(&sem_klienci, 0, 0);

    printf("Salon fryzjerski otwarty!\n");

    pthread_create(&manager, NULL, kierownik, NULL);

    // Tworzenie wątków fryzjerów
    for (int i = 0; i < LICZBA_FRYZJERÓW; i++) {
        if (pthread_create(&fryzjerzy[i], NULL, fryzjer, (void *)(long)i) != 0) {
            perror("Błąd tworzenia fryzjera");
        }
    }

    // Tworzenie wątków klientów
    for (int i = 0; i < LICZBA_KLIENTÓW; i++) {
        if (pthread_create(&klienci[i], NULL, klient, (void *)(long)i) != 0) {
            perror("Błąd tworzenia klienta");
        }
        usleep(10000); // Zapobiega nagłemu obciążeniu systemu
    }

    // Oczekiwanie na zakończenie wszystkich klientów
    for (int i = 0; i < LICZBA_KLIENTÓW; i++) {
        pthread_join(klienci[i], NULL);
    }

    // Oczekiwanie na zakończenie fryzjerów
    for (int i = 0; i < LICZBA_FRYZJERÓW; i++) {
        pthread_join(fryzjerzy[i], NULL);
    }

    pthread_join(manager, NULL);

    sem_destroy(&sem_fotele);
    sem_destroy(&sem_klienci);

    pthread_mutex_destroy(&mutex_kasa);
    pthread_mutex_destroy(&mutex_kolejka);

    printf("Stan kasy na koniec dnia: %d zł\n", kasa);
    printf("Salon fryzjerski zamyka się!\n");

    return 0;
}
