#include "wspolne.h"
#include <sys/wait.h>
#include <sys/prctl.h>

void kierownik_process(int czas_symulacji) {
    prctl(PR_SET_NAME, "kierownik", 0, 0, 0);

    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    
    int shm_fd = shm_open(SHM_NAZWA, O_RDWR, 0666);
    if(shm_fd == -1) {
        perror("kierownik shm_open");
        exit(EXIT_FAILURE);
    }
    wspolne_t *wspolne = mmap(NULL, sizeof(wspolne_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(wspolne == MAP_FAILED) {
        perror("kierownik mmap");
        exit(EXIT_FAILURE);
    }
    
    sleep(czas_symulacji);
    printf("Kierownik: Czas pracy salonu minimal. Rozpoczynam zamykanie salonu.\n");
    
    wspolne->salon_otwarty = 0;
    
    sem_t *sem_kolejka_mutex = sem_open(SEM_KOLEJKA_MUTEX, 0);
    if(sem_wait(sem_kolejka_mutex) == -1) {
        perror("kierownik sem_wait(sem_kolejka_mutex)");
    }
    for (int i = 0; i < wspolne->liczba_klientow; i++) {
        int index = (wspolne->glowa_kolejki + i) % wspolne->rozmiar_kolejki;
        pid_t klient_pid = wspolne->kolejka[index].pid;
        if(kill(klient_pid, SIGUSR2) == -1) {
            perror("kierownik kill(SIGUSR2) do klienta");
        }
    }
    sem_post(sem_kolejka_mutex);
    
    for (int i = 0; i < wspolne->liczba_fryzjerow; i++) {
        pid_t fryzjer_pid = wspolne->fryzjerowie[i];
        if(kill(fryzjer_pid, SIGUSR1) == -1) {
            perror("kierownik kill(SIGUSR1) do fryzjera");
        }
    }
    printf("Kierownik: Wyslano polecenia zakonczenia pracy.\n");
    
    int status;
    while(wait(&status) > 0);
    
    munmap(wspolne, sizeof(wspolne_t));
    close(shm_fd);
}
