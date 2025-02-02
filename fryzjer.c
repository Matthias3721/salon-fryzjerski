#include "wspolne.h"
#include <sys/time.h>
#include <sys/prctl.h>

volatile sig_atomic_t zakonczyc = 0;

void obsluga_sygnalu(int sig) {
    // SIGUSR1 - polecenie zakonczenia pracy fryzjera
    zakonczyc = 1;
}

void fryzjer_process(int fryzjer_nr) {
    // Ustawienie obsługi SIGUSR1
    struct sigaction sa;
    sa.sa_handler = obsluga_sygnalu;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction fryzjera");
        exit(EXIT_FAILURE);
    }
    signal(SIGUSR2, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    
    // Ustawienie nazwy procesu na "fryzjer"
    prctl(PR_SET_NAME, "fryzjer", 0, 0, 0);
    
    int shm_fd = shm_open(SHM_NAZWA, O_RDWR, 0666);
    if(shm_fd == -1) {
        perror("fryzjer shm_open");
        exit(EXIT_FAILURE);
    }
    wspolne_t *wspolne = mmap(NULL, sizeof(wspolne_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(wspolne == MAP_FAILED) {
        perror("fryzjer mmap");
        exit(EXIT_FAILURE);
    }
    
    sem_t *sem_kolejka_mutex = sem_open(SEM_KOLEJKA_MUTEX, 0);
    sem_t *sem_klientow_czeka = sem_open(SEM_KLIENTOW_CZEKA, 0);
    sem_t *sem_kasa = sem_open(SEM_KASA, 0);
    if(sem_kolejka_mutex == SEM_FAILED || sem_klientow_czeka == SEM_FAILED ||
       sem_kasa == SEM_FAILED) {
        perror("fryzjer sem_open");
        exit(EXIT_FAILURE);
    }
    
    printf("Fryzjer [%d]: rozpoczyna prace.\n", fryzjer_nr);
    
    struct timespec ts;
    while(!zakonczyc) {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;
        int s = sem_timedwait(sem_klientow_czeka, &ts);
        if(s == -1) {
            if(errno == ETIMEDOUT) {
                if(wspolne->salon_otwarty == 0)
                    break;
                continue;
            } else {
                perror("fryzjer sem_timedwait");
                break;
            }
        }
        
        // Pobieramy klienta z kolejki
        if(sem_wait(sem_kolejka_mutex) == -1) {
            perror("fryzjer sem_wait(sem_kolejka_mutex)");
            continue;
        }
        if(wspolne->liczba_klientow <= 0) {
            sem_post(sem_kolejka_mutex);
            continue;
        }
        int index = wspolne->glowa_kolejki;
        pid_t klient_pid = wspolne->kolejka[index].pid;
        int client_num = wspolne->kolejka[index].numer_klienta;
        int payment = wspolne->kolejka[index].wplata;
        char payment_desc[64];
        strncpy(payment_desc, wspolne->kolejka[index].payment_desc, sizeof(payment_desc));
        payment_desc[sizeof(payment_desc)-1] = '\0';
        wspolne->glowa_kolejki = (wspolne->glowa_kolejki + 1) % wspolne->rozmiar_kolejki;
        wspolne->liczba_klientow--;
        sem_post(sem_kolejka_mutex);
        
        printf("Fryzjer [%d]: obsluguje klienta [%d].\n", fryzjer_nr, client_num);
        
        // Obsługa płatności
        if(sem_wait(sem_kasa) == -1) {
            perror("fryzjer sem_wait(sem_kasa)");
        }
        if(payment == 30) {
            // Opcja A: płaci dokładnie 30 zł – dodajemy trzy banknoty 10 zł
            wspolne->banknot10 += 3;
            printf("Klient [%d]: wplaca 30 (%s), brak reszty.\n", client_num, payment_desc);
        } else if(payment == 50) {
            // Opcja B: płaci 50 zł – dodajemy banknot 50, a wydajemy 20 zł reszty
            wspolne->banknot50 += 1;
            if(wspolne->banknot20 > 0) {
                wspolne->banknot20 -= 1;
                printf("Klient [%d]: wplaca 50 (%s), otrzymuje reszte 20.\n", client_num, payment_desc);
            } else {
                printf("Klient [%d]: wplaca 50 (%s), brak reszty!\n", client_num, payment_desc);
            }
        } else if(payment == 100) {
            // Opcja C: płaci 100 zł – dodajemy banknot 100, a wydajemy resztę 70 = (50+20)
            wspolne->banknot100 += 1;
            if(wspolne->banknot50 > 0 && wspolne->banknot20 > 0) {
                wspolne->banknot50 -= 1;
                wspolne->banknot20 -= 1;
                printf("Klient [%d]: wplaca 100 (%s), otrzymuje reszte 50+20.\n", client_num, payment_desc);
            } else {
                printf("Klient [%d]: wplaca 100 (%s), brak reszty!\n", client_num, payment_desc);
            }
        } else {
            printf("Klient [%d]: wplaca %d, nieznana opcja.\n", client_num, payment);
        }
        sem_post(sem_kasa);
        
        // Symulacja wykonania usługi (strzyżenia) – stały czas 1 sekunda
        sleep(1);
        printf("Fryzjer [%d]: zakonczyl obsluge klienta [%d] (czas: 1 s).\n", fryzjer_nr, client_num);
        
        // Informujemy klienta o zakończeniu usługi
        if(kill(klient_pid, SIGALRM) == -1) {
            perror("fryzjer kill(SIGALRM)");
        }
    }
    
    printf("Fryzjer [%d]: konczy prace.\n", fryzjer_nr);
    
    sem_close(sem_kolejka_mutex);
    sem_close(sem_klientow_czeka);
    sem_close(sem_kasa);
    munmap(wspolne, sizeof(wspolne_t));
    close(shm_fd);
}
