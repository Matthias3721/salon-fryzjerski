#include "salon.h"
#include <sys/wait.h>


int shm_id;
ShmSalon *salon_dane;
int semid;

// Obsluga sygnalu SIGNINT
void sigint_handler(int signum) {
    printf("\nOtrzymano SIGINT! Zamykanie salonu...\n");
    salon_dane->salon_otwarty = 0;
    
    // powiadomienie fryzjerow o zamknięciu
    for (int i = 0; i < LICZBA_FRYZJERÓW; i++) {
        semop(semid, &(struct sembuf){ SEM_KLIENCI, 1, 0 }, 1);
    }

    cleanup_ipc();
    exit(0);
}

// Funkcja inicjalizujaca mechanizmy IPC
void setup_ipc() {
    key_t key = ftok(".", 'X');                                     // Tworzy unikalny klucz do mechanizmow IPC
    shm_id = shmget(key, sizeof(ShmSalon), IPC_CREAT | 0666);       // Tworzy segment pamieci współdzielonej
    if (shm_id == -1) {
        perror("Błąd tworzenia pamięci współdzielonej");
        exit(1);
    }

    // przylacza pamięć wspoldzielona do przestrzeni adresowej procesu
    salon_dane = (ShmSalon *)shmat(shm_id, NULL, 0);
    if (salon_dane == (void *)-1) {
        perror("Błąd przydzielania pamięci współdzielonej");
        exit(1);
    }

    salon_dane->kasa = 100;
    salon_dane->klienci_w_poczekalni = 0;
    salon_dane->salon_otwarty = 1;

    semid = semget(key, 4, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("Błąd tworzenia semaforów");
        exit(1);
    }

    // Inicjalizacja wartości semaforów
    semctl(semid, SEM_FOTELE, SETVAL, LICZBA_FOTELI);            // Semafor określający liczbę dostępnych foteli
    semctl(semid, SEM_KLIENCI, SETVAL, 0);                       // Semafor oczekujących klientów (początkowo 0)
    semctl(semid, SEM_KASA, SETVAL, 1);                          // Mutex do synchronizacji dostępu do kasy
    semctl(semid, SEM_MUTEX_KOLEJKA, SETVAL, 1);                 // Mutex do synchronizacji dostępu do kolejki klientów

}

void cleanup_ipc() {
    shmdt(salon_dane);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    printf("IPC usunięte poprawnie.\n");
}

// Funkcja kierownika
void kierownik() {
    sleep(10);
    printf("Kierownik zamyka salon!\n");
    salon_dane->salon_otwarty = 0;                                // Ustawia flagę oznaczającą zamknięcie salonu

    for (int i = 0; i < LICZBA_FRYZJERÓW; i++) {                  // informuje fryzjerów, aby zakończyli pracę
        semop(semid, &(struct sembuf){ SEM_KLIENCI, 1, 0 }, 1);
    }

    printf("Kasa: %d zł\n", salon_dane->kasa);
    cleanup_ipc();
    exit(0);
}

int main() {
    signal(SIGINT, sigint_handler);
    setup_ipc();

    printf("Salon fryzjerski otwarty!\n");

    // Tworzenie procesu dla kierownika
    pid_t kierownik_pid = fork();
    if (kierownik_pid == 0) {
        kierownik();
    }

    // Tworzenie procesow fryzjerow
    for (int i = 0; i < LICZBA_FRYZJERÓW; i++) {
    pid_t pid = fork();
    if (pid == 0) { // Proces potomny - fryzjer
        fryzjer(i); // Przekazujemy i jako ID fryzjera
        exit(0);
        }
    }

    // Tworzenie procesow klientow
    for (int i = 0; i < LICZBA_KLIENTÓW; i++) {
    pid_t pid = fork();
    if (pid == 0) { // Proces potomny - klient
        klient(i); // Przekazujemy i jako ID klienta
        exit(0);
        }
    }


    for (int i = 0; i < LICZBA_KLIENTÓW + LICZBA_FRYZJERÓW + 1; i++) {
        wait(NULL);
    }

    printf("Salon fryzjerski zamknięty!\n");
    return 0;
}
