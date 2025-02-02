#include "wspolne.h"
#include <sys/prctl.h>
#include <time.h>

volatile sig_atomic_t usluga_zakonczona = 0;
static int klient_nr = 0; // globalny numer klienta wykorzystywany przez obsługę sygnałów

void obsluga_uslugi(int sig) {
    // SIGALRM - usługa zakończona
    usluga_zakonczona = 1;
}

void opusc_salon(int sig) {
    // SIGUSR2 - natychmiastowe opuszczenie salonu
    printf("Klient [%d]: otrzymalem polecenie opuszczenia salonu!\n", klient_nr);
    exit(EXIT_SUCCESS);
}

void klient_process() {
    prctl(PR_SET_NAME, "klient", 0, 0, 0);

    // Seed generatora losowego – mieszamy czas i PID
    srand((unsigned int)(time(NULL) ^ getpid()));

    struct sigaction sa;
    sa.sa_handler = obsluga_uslugi;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("klient sigaction(SIGALRM)");
        exit(EXIT_FAILURE);
    }
    signal(SIGUSR2, opusc_salon);
    signal(SIGUSR1, SIG_IGN);
    
    int shm_fd = shm_open(SHM_NAZWA, O_RDWR, 0666);
    if(shm_fd == -1) {
        perror("klient shm_open");
        exit(EXIT_FAILURE);
    }
    wspolne_t *wspolne = mmap(NULL, sizeof(wspolne_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(wspolne == MAP_FAILED) {
        perror("klient mmap");
        exit(EXIT_FAILURE);
    }
    
    sem_t *sem_kolejka_mutex = sem_open(SEM_KOLEJKA_MUTEX, 0);
    sem_t *sem_klientow_czeka = sem_open(SEM_KLIENTOW_CZEKA, 0);
    if(sem_kolejka_mutex == SEM_FAILED || sem_klientow_czeka == SEM_FAILED) {
        perror("klient sem_open");
        exit(EXIT_FAILURE);
    }
    
    if(sem_wait(sem_kolejka_mutex) == -1) {
        perror("klient sem_wait(sem_kolejka_mutex)");
        exit(EXIT_FAILURE);
    }
    klient_nr = wspolne->next_client_number;
    wspolne->next_client_number++;
    sem_post(sem_kolejka_mutex);
    
    if(wspolne->salon_otwarty == 0) {
        printf("Klient [%d]: Salon zamkniety - wychodze.\n", klient_nr);
        munmap(wspolne, sizeof(wspolne_t));
        close(shm_fd);
        exit(EXIT_SUCCESS);
    }
    
    // Losujemy opcję płatności
    int option = rand() % 3;  // 0, 1 lub 2
    int payment;
    char payment_desc[64];
    if(option == 0) {
        payment = 30;
        strcpy(payment_desc, "10x3");
    } else if(option == 1) {
        payment = 50;
        strcpy(payment_desc, "50x1");
    } else {
        payment = 100;
        strcpy(payment_desc, "100x1");
    }
    
    // Rezerwacja miejsca w poczekalni
    if(sem_wait(sem_kolejka_mutex) == -1) {
        perror("klient sem_wait(sem_kolejka_mutex)");
        exit(EXIT_FAILURE);
    }
    if(wspolne->liczba_klientow >= wspolne->rozmiar_kolejki) {
        sem_post(sem_kolejka_mutex);
        printf("Klient [%d]: brak miejsca w poczekalni - wychodze.\n", klient_nr);
        munmap(wspolne, sizeof(wspolne_t));
        close(shm_fd);
        exit(EXIT_SUCCESS);
    }
    int index = wspolne->ogon_kolejki;
    wspolne->kolejka[index].pid = getpid();
    wspolne->kolejka[index].wplata = payment;
    wspolne->kolejka[index].numer_klienta = klient_nr;
    strncpy(wspolne->kolejka[index].payment_desc, payment_desc, sizeof(wspolne->kolejka[index].payment_desc));
    wspolne->kolejka[index].payment_desc[sizeof(wspolne->kolejka[index].payment_desc)-1] = '\0';
    wspolne->ogon_kolejki = (wspolne->ogon_kolejki + 1) % wspolne->rozmiar_kolejki;
    wspolne->liczba_klientow++;
    sem_post(sem_kolejka_mutex);
    
    sem_post(sem_klientow_czeka);
    printf("Klient [%d]: zajal miejsce w poczekalni i czeka na obsluge. (Wplaca %d, %s)\n", klient_nr, payment, payment_desc);
    
    while(!usluga_zakonczona)
        pause();
    
    printf("Klient [%d]: usluga zakonczona - opuszczam salon.\n", klient_nr);
    
    munmap(wspolne, sizeof(wspolne_t));
    close(shm_fd);
}
