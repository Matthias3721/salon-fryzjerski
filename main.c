#include "wspolne.h"
#include <sys/wait.h>
#include <string.h>
#include <sys/prctl.h>  // do ustawiania nazwy procesu

// Deklaracje funkcji z pozostałych modułów
void fryzjer_process(int fryzjer_nr);
void klient_process();
void kierownik_process(int czas_symulacji);

int main(int argc, char *argv[]) {
    // Ustawienie nazwy procesu na "glowny"
    prctl(PR_SET_NAME, "glowny", 0, 0, 0);

    int liczba_fryzjerow = LICZBA_FRYZJEROW;
    int pojemnosc_poczekalni = POJEMNOSC_POCZEKALNI;
    int czas_symulacji = CZAS_SYMULACJI;
    int liczba_klientow = LICZBA_KLIENTOW; // domyślna liczba klientów

    // Jeśli jako pierwszy argument podamy liczbę klientów, używamy jej
    if(argc >= 2) {
        liczba_klientow = atoi(argv[1]);
        if(liczba_klientow <= 0) {
            fprintf(stderr, "Podaj dodatnia liczbe klientow.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Tworzymy obszar pamięci współdzielonej
    int shm_fd = shm_open(SHM_NAZWA, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if(ftruncate(shm_fd, sizeof(wspolne_t)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    wspolne_t *wspolne = mmap(NULL, sizeof(wspolne_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(wspolne == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    
    // Inicjalizacja danych w pamięci współdzielonej
    wspolne->banknot10 = 1000;
    wspolne->banknot20 = 1000;  // więcej banknotów 20, aby ułatwić wydawanie reszty
    wspolne->banknot50 = 1000;
    wspolne->banknot100 = 0;
    wspolne->glowa_kolejki = 0;
    wspolne->ogon_kolejki = 0;
    wspolne->liczba_klientow = 0;
    wspolne->rozmiar_kolejki = pojemnosc_poczekalni;
    wspolne->liczba_fryzjerow = 0;
    wspolne->salon_otwarty = 1;
    wspolne->next_client_number = 1;
    
    // Tworzymy semafory
    sem_t *sem_kolejka_mutex = sem_open(SEM_KOLEJKA_MUTEX, O_CREAT, 0666, 1);
    if(sem_kolejka_mutex == SEM_FAILED) {
        perror("sem_open sem_kolejka_mutex");
        exit(EXIT_FAILURE);
    }
    sem_t *sem_klientow_czeka = sem_open(SEM_KLIENTOW_CZEKA, O_CREAT, 0666, 0);
    if(sem_klientow_czeka == SEM_FAILED) {
        perror("sem_open sem_klientow_czeka");
        exit(EXIT_FAILURE);
    }
    sem_t *sem_kasa = sem_open(SEM_KASA, O_CREAT, 0666, 1);
    if(sem_kasa == SEM_FAILED) {
        perror("sem_open sem_kasa");
        exit(EXIT_FAILURE);
    }
    
    // Tworzymy procesy fryzjerów
    pid_t pid;
    for (int i = 0; i < liczba_fryzjerow; i++) {
        pid = fork();
        if(pid < 0) {
            perror("fork fryzjera");
            exit(EXIT_FAILURE);
        } else if(pid == 0) {
            int my_fryzjer_nr = i + 1;
            fryzjer_process(my_fryzjer_nr);
            exit(EXIT_SUCCESS);
        } else {
            wspolne->fryzjerowie[i] = pid;
            wspolne->liczba_fryzjerow++;
        }
    }
    
    // Tworzymy proces kierownika (uruchamiamy go równolegle)
    pid = fork();
    if(pid < 0) {
        perror("fork kierownika");
        exit(EXIT_FAILURE);
    } else if(pid == 0) {
        kierownik_process(czas_symulacji);
        exit(EXIT_SUCCESS);
    }
    
    // Pętla tworząca klientów – działamy tylko przez czas symulacji
    time_t start = time(NULL);
    int client_count = 0;
    while(client_count < liczba_klientow && difftime(time(NULL), start) < czas_symulacji) {
        pid = fork();
        if(pid < 0) {
            perror("fork klienta");
        } else if(pid == 0) {
            klient_process();
            exit(EXIT_SUCCESS);
        }
        client_count++;
        usleep(10000);
    }
    
    // Po upływie czasu symulacji zamykamy salon
    wspolne->salon_otwarty = 0;
    printf("Glowny: Salon zostal zamkniety dla nowych klientow.\n");
    
    // Oczekujemy na zakończenie procesów dzieci
    int status;
    while(wait(&status) > 0);
    
    // Obliczamy łączną kwotę w kasie
    int suma = wspolne->banknot10 * 10 + wspolne->banknot20 * 20 + wspolne->banknot50 * 50 + wspolne->banknot100 * 100;
    printf("Stan kasy:\n");
    printf("  banknot10: %d\n", wspolne->banknot10);
    printf("  banknot20: %d\n", wspolne->banknot20);
    printf("  banknot50: %d\n", wspolne->banknot50);
    printf("  banknot100: %d\n", wspolne->banknot100);
    printf("Lacznie: %d zl\n", suma);
    
    // Sprzątamy: zamykamy i usuwamy semafory oraz pamięć współdzieloną
    sem_close(sem_kolejka_mutex);
    sem_unlink(SEM_KOLEJKA_MUTEX);
    sem_close(sem_klientow_czeka);
    sem_unlink(SEM_KLIENTOW_CZEKA);
    sem_close(sem_kasa);
    sem_unlink(SEM_KASA);
    
    munmap(wspolne, sizeof(wspolne_t));
    shm_unlink(SHM_NAZWA);
    
    return 0;
}
