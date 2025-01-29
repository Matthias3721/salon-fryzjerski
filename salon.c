#include "salon.h"
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>

// Globalne zmienne IPC
ShmSalon *salon_dane;
int shm_id;
int sem_fotele;
int sem_klienci;
int sem_mutex_kasa;
int sem_mutex_kolejka;

void setup_ipc() {
    key_t shm_key = ftok("salon", 1);
    shm_id = shmget(shm_key, sizeof(ShmSalon), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Błąd przy tworzeniu pamięci współdzielonej");
        exit(1);
    }

    salon_dane = (ShmSalon *)shmat(shm_id, NULL, 0);
    if (salon_dane == (void *)-1) {
        perror("Błąd przy przypisaniu pamięci współdzielonej");
        exit(1);
    }

    // Inicjalizacja wartości początkowych
    salon_dane->kasa = 100;
    salon_dane->klienci_w_poczekalni = 0;
    salon_dane->salon_otwarty = 1;

    // Tworzenie semaforów
    sem_fotele = semget(ftok("salon", 2), 1, IPC_CREAT | 0666);
    semctl(sem_fotele, 0, SETVAL, LICZBA_FOTELI);

    sem_klienci = semget(ftok("salon", 3), 1, IPC_CREAT | 0666);
    semctl(sem_klienci, 0, SETVAL, 0);

    sem_mutex_kasa = semget(ftok("salon", 4), 1, IPC_CREAT | 0666);
    semctl(sem_mutex_kasa, 0, SETVAL, 1);

    sem_mutex_kolejka = semget(ftok("salon", 5), 1, IPC_CREAT | 0666);
    semctl(sem_mutex_kolejka, 0, SETVAL, 1);
}

void cleanup_ipc() {
    shmdt(salon_dane);
    shmctl(shm_id, IPC_RMID, NULL);

    semctl(sem_fotele, 0, IPC_RMID);
    semctl(sem_klienci, 0, IPC_RMID);
    semctl(sem_mutex_kasa, 0, IPC_RMID);
    semctl(sem_mutex_kolejka, 0, IPC_RMID);
}
