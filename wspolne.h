// Umiesc te dwie linie na samym początku pliku, jeszcze przed wszelkimi #include
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#ifndef WSPOLNE_H
#define WSPOLNE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>

// USTAWIENIA PROJEKTU (MOŻNA EDYTOWAĆ)
#define LICZBA_FRYZJEROW     500      // liczba fryzjerów (F > 1)
#define POJEMNOSC_POCZEKALNI 3000     // liczba miejsc w poczekalni
#define CZAS_SYMULACJI       30     // czas symulacji (w sekundach)
#define LICZBA_KLIENTOW      1000     // domyślna liczba klientów

// Nazwy obiektów pamięci współdzielonej i semaforów
#define SHM_NAZWA           "/salon_shm"
#define SEM_KOLEJKA_MUTEX   "/sem_kolejka_mutex"
#define SEM_KLIENTOW_CZEKA  "/sem_klientow_czeka"
#define SEM_KASA            "/sem_kasa"

// Maksymalne wartości
#define MAX_POLECZALNIA     3000  // maksymalna pojemność poczekalni
#define MAX_FRYZJEROW       5000  // maksymalna liczba fryzjerów

// Definicje kwot
#define KOSZT_USLUGI   30       // koszt usługi
// Opcje płatności: klient może wpłacić 30, 50 lub 100 zł.
 
// Struktura danych umieszczanych w pamięci współdzielonej
typedef struct {
    int banknot10;
    int banknot20;
    int banknot50;
    int banknot100;
    
    int glowa_kolejki;
    int ogon_kolejki;
    int liczba_klientow;
    int rozmiar_kolejki;  // pojemność poczekalni = POJEMNOSC_POCZEKALNI
    struct {
        pid_t pid;          // PID klienta (do sygnalizacji)
        int wplata;         // kwota wpłacona przez klienta (30, 50 lub 100)
        int numer_klienta;  // przydzielony numer klienta
        char payment_desc[64];  // opis banknotów, np. "10x3" lub "50x1" lub "100x1"
    } kolejka[MAX_POLECZALNIA];
    
    int liczba_fryzjerow;
    pid_t fryzjerowie[MAX_FRYZJEROW];
    
    int salon_otwarty;   // 1 - otwarty, 0 - zamykany
    
    int next_client_number;  // kolejny numer klienta
} wspolne_t;

#endif
