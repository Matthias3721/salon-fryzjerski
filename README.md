RAPORT PROJEKTU: Symulacja Salonu Fryzjerskiego
Autor: Mateusz Więcek
Repozytorium GitHub: https://github.com/Matthias3721/salon-fryzjerski
________________________________________
1. Wstęp
Celem projektu było stworzenie symulacji salonu fryzjerskiego opartej na procesach. Projekt opiera się na klasycznym problemie „śpiącego fryzjera” i został rozbudowany o zaawansowaną obsługę płatności, gdzie klienci losowo wybierają jedną z trzech opcji wpłaty (30 zł, 50 zł lub 100 zł). Na podstawie wpłaconej kwoty system automatycznie oblicza i wydaje resztę (np. przy wpłacie 50 zł – reszta 20 zł; przy wpłacie 100 zł – reszta 70 zł, wydawana jako banknoty 50 i 20 zł). Zrezygnowano z dodatkowej obsługi foteli, skupiając się na modelu, w którym klienci korzystają z ograniczonej liczby miejsc w poczekalni. Synchronizacja między procesami została rozwiązana przy użyciu pamięci współdzielonej i semaforów, co gwarantuje stabilność działania systemu nawet przy bardzo dużej liczbie procesów.
________________________________________
2. Opis implementacji
Główne elementy programu:
•	Fryzjerzy:
Procesy reprezentujące fryzjerów, którzy:
o	Pobierają klientów z kolejki (poczekalni) zdefiniowanej w pamięci współdzielonej.
o	Obsługują płatności, przyjmując różne kwoty wpłat (30 zł, 50 zł lub 100 zł) wraz z opisem sposobu wpłaty.
o	Wydają resztę – np. przy wpłacie 50 zł klient otrzymuje 20 zł, a przy wpłacie 100 zł – 50 zł i 20 zł, jeśli kasa dysponuje odpowiednimi banknotami.
o	Symulują strzyżenie przez stały czas 1 sekundy.
•	Klienci:
Procesy reprezentujące klientów, którzy:
o	Losowo wybierają jedną z trzech opcji płatności (30 zł, 50 zł lub 100 zł) oraz zapisują opis wpłaty (np. „10x3”, „50x1”, „100x1”).
o	Starają się zająć miejsce w poczekalni (ograniczonej liczby miejsc, zdefiniowanej przez makro POJEMNOSC_POCZEKALNI).
o	W przypadku braku miejsca opuszczają salon.
o	Czekają na zakończenie usługi, sygnalizowane przez fryzjera.
•	Poczekalnia:
Reprezentowana jest przez tablicę w pamięci współdzielonej o rozmiarze określonym przez makro MAX_POCZEKALNIA. Klienci, którzy nie znajdą wolnego miejsca, opuszczają salon.
•	Kasa:
Stan kasy przechowywany jest w pamięci współdzielonej (struktura wspolne_t) i obejmuje liczbę banknotów 10, 20, 50 oraz 100 zł. Płatności klientów aktualizują stan kasy – dodawane są banknoty wpłacone przez klientów, a wydawane są banknoty jako reszta.
Mechanizmy synchronizacji:
•	Pamięć współdzielona:
Funkcje shm_open oraz mmap służą do utworzenia i mapowania obszaru pamięci współdzielonej, w którym przechowywany jest stan salonu (kolejka, kasa, zmienne sterujące).
•	Semafory:
Semafory (np. SEM_KOLEJKA_MUTEX, SEM_KLIENTOW_CZEKA, SEM_KASA) zarządzają dostępem do krytycznych sekcji pamięci współdzielonej, zapewniając, że operacje na kolejce i kasie są wykonywane w sposób atomowy.
•	Obsługa przerwanych wywołań:
W operacjach sem_wait dodano pętle, które obsługują przerwania (EINTR), co minimalizuje ryzyko błędów synchronizacyjnych.
Zmiany w stosunku do klasycznego problemu:
•	Zaimplementowano losowe wybieranie sposobu płatności przez klientów oraz mechanizm wydawania reszty, co dodaje nową funkcjonalność do klasycznego problemu „śpiącego fryzjera”.
•	Usunięto obsługę foteli, upraszczając model do systemu poczekalni, fryzjerów i klientów.
•	Zoptymalizowano czasy symulacji (strzyżenie trwa 1 sekundę, a opóźnienie między przybyciem klientów można łatwo zmodyfikować).
________________________________________
3. Opis testów
TEST 1: Mały salon
Konfiguracja:
•	LICZBA_FRYZJERÓW: 2
•	LICZBA_KLIENTÓW: 10
•	POJEMNOSC_POCZEKALNI: 3
Oczekiwany wynik:
•	Fryzjerzy obsługują klientów kolejno; część klientów opuszcza salon, gdy poczekalnia jest pełna.
•	System wyświetla komunikaty o przyjętych wpłatach (wraz z opisem, np. „10x3”, „50x1”, „100x1”) oraz o wydawaniu odpowiedniej reszty lub braku reszty.
Wynik testu:
•	Symulacja działa zgodnie z oczekiwaniami – klienci wchodzą do poczekalni, fryzjerzy obsługują ich, a kasa jest aktualizowana zgodnie z wpłatami.
Wynik testu:
•	Obciążenie CPU: bliskie zeru, z minimalnymi skokami do 0,7% na rdzeniach.
•	Zużycie RAM: około 324 MB z dostępnych 7,73 GB.
Symulacja działa zgodnie z oczekiwaniami – klienci wchodzą do poczekalni, fryzjerzy obsługują ich, a kasa jest aktualizowana zgodnie z wpłatami.
 
 ![image](https://github.com/user-attachments/assets/0128e381-8c74-4ff6-bbf1-55063ada76c8)

________________________________________

TEST 2: Duży salon
Konfiguracja:
•	LICZBA_FRYZJERÓW: 10
•	LICZBA_KLIENTÓW: 50
•	POJEMNOSC_POCZEKALNI: 20
Oczekiwany wynik:
•	Wielu fryzjerów obsługuje klientów równolegle, skracając kolejkę.
•	Większość klientów znajduje miejsce, a komunikaty pokazują szczegółowe informacje o wpłatach i wydawaniu reszty.
Wynik testu:
•	Salon działa szybko, a system poprawnie realizuje transakcje, wydając resztę tam, gdzie to możliwe.
Wynik testu:
•	Obciążenie CPU: minimalne, sporadyczne skoki do 0,7% na wybranych rdzeniach.
•	Zużycie RAM: około 334 MB z 7,73 GB dostępnych.
Salon działa szybko, a system poprawnie realizuje transakcje, wydając resztę tam, gdzie to możliwe
![image](https://github.com/user-attachments/assets/d16839f1-0a6b-4131-a323-4a0353b313ff)

________________________________________
TEST 3: Zbyt wielu klientów
Konfiguracja:
•	LICZBA_FRYZJERÓW: 3
•	LICZBA_KLIENTÓW: 100
•	POJEMNOSC_POCZEKALNI: 5
Oczekiwany wynik:
•	Duża liczba klientów powoduje długą kolejkę w poczekalni.
•	Po zapełnieniu miejsc wielu klientów opuszcza salon.
•	Kasa rośnie wolniej, ponieważ nie wszyscy klienci są obsłużeni.
Wynik testu:
•	Większość klientów opuszcza salon z powodu braku miejsc, co skutkuje mniejszym wzrostem stanu kasy, zgodnie z założeniami.
Wynik testu:
•	Obciążenie CPU: wzrosło do 3,3% na jednym z rdzeni.
•	Zużycie RAM: około 347 MB z 7,73 GB.
Większość klientów opuszcza salon z powodu braku miejsc, co skutkuje mniejszym wzrostem stanu kasy, zgodnie z założeniami.
![image](https://github.com/user-attachments/assets/9db5e53f-446b-425b-a01f-2deb91975135)

________________________________________
TEST 4: Maksymalna liczba procesów
Konfiguracja:
•	LICZBA_FRYZJERÓW: 100
•	LICZBA_KLIENTÓW: 2000
•	POJEMNOSC_POCZEKALNI: 50
Oczekiwany wynik:
•	System operacyjny może obsłużyć dużą liczbę procesów (przy odpowiednich limitach systemowych).
•	CPU i pamięć RAM mogą być obciążone, ale symulacja działa stabilnie.
•	Komunikaty wyświetlają szczegółowe informacje o wpłatach, wydawaniu reszty oraz końcowym stanie kasy.
Wynik testu:
•	Symulacja działa poprawnie, a system nie osiąga limitu procesów. Wydajność jest zadowalająca, a stan kasy na końcu dokładnie odzwierciedla przeprowadzone transakcje.\
Wynik testu:
•	Obciążenie CPU: wzrosło do 5,5% na jednym rdzeniu i do około 4,5% na innym.
•	Zużycie RAM: około 388 MB z 7,73 GB dostępnych.
Symulacja działa poprawnie, a system nie osiąga limitu procesów. Wydajność jest zadowalająca, a stan kasy na końcu dokładnie odzwierciedla przeprowadzone transakcje.
 ![image](https://github.com/user-attachments/assets/7c4d7073-4340-4ca5-a672-ec8786a5ca47)

________________________________________
4. Wnioski
•	Poprawność funkcjonalna:
Testy wykazały, że system poprawnie obsługuje różne opcje płatności i wydawanie reszty – klient, który wpłaci 50 zł, otrzymuje 20 zł reszty, a przy wpłacie 100 zł, klient powinien otrzymać 50 zł i 20 zł, o ile kasa dysponuje odpowiednimi banknotami.
•	Synchronizacja:
Użycie pamięci współdzielonej i semaforów (SEM_KOLEJKA_MUTEX, SEM_KLIENTOW_CZEKA, SEM_KASA) zapewnia spójny dostęp do danych, eliminując problemy z równoczesnym dostępem do stanu salonu.
•	Skalowalność:
System może być skonfigurowany do obsługi bardzo dużej liczby klientów i fryzjerów, jednak największym ograniczeniem pozostaje liczba miejsc w poczekalni, co wpływa na liczbę obsłużonych transakcji.
•	Wydajność:
Nawet przy dużej liczbie procesów system działa stabilnie, a obciążenie CPU i pamięci RAM pozostaje na akceptowalnym poziomie (należy jednak pamiętać o limitach systemowych dla procesów).
________________________________________
5. Kluczowe fragmenty kodu
•	Tworzenie i obsługa pamięci współdzielonej
https://github.com/Matthias3721/salon-fryzjerski/blob/5bfe0f71b3dccbc193c1f0b1bff090b3c9013c7a/wspolne.h#L2C1-L66C7
Ten fragment definiuje wszystkie dane współdzielone przez procesy: stan kasy, kolejkę klientów oraz listę fryzjerów. Ustawienia takie jak liczba fryzjerów, poczekalnia, czas symulacji i liczba klientów są skonfigurowane tutaj.
•	Tworzenie procesów (fork, exit, wait)
https://github.com/Matthias3721/salon-fryzjerski/blob/5bfe0f71b3dccbc193c1f0b1bff090b3c9013c7a/main.c#L75C5-L115C6
Powyższy fragment pokazuje, jak za pomocą funkcji fork() tworzone są procesy fryzjerów, klientów oraz kierownika. W procesie rodzica używane jest wait(), aby poczekać na zakończenie wszystkich procesów potomnych.
•	Obsługa sygnałów (sigaction, signal, kill)
https://github.com/Matthias3721/salon-fryzjerski/blob/5bfe0f71b3dccbc193c1f0b1bff090b3c9013c7a/fryzjer.c#L14C5-L23C30
Ten fragment kodu ustawia obsługę sygnału SIGUSR1 dla fryzjera przy pomocy funkcji sigaction(), co umożliwia fryzjerowi odpowiednią reakcję (ustawienie flagi zakończenia pracy). Dodatkowo, inne sygnały, które nie dotyczą fryzjera, są ignorowane.
•	Synchronizacja przy użyciu semaforów
https://github.com/Matthias3721/salon-fryzjerski/blob/5bfe0f71b3dccbc193c1f0b1bff090b3c9013c7a/main.c#L59C4-L73C6
Ten fragment pokazuje, jak tworzone są semafory, które synchronizują dostęp do kolejki klientów (SEM_KOLEJKA_MUTEX), sygnalizują obecność klientów (SEM_KLIENTOW_CZEKA) oraz chronią operacje na kasie (SEM_KASA).

________________________________________
6. Możliwe ulepszenia
•	Dynamiczne skalowanie fryzjerów:
Wprowadzenie mechanizmu automatycznego zwiększania liczby fryzjerów w zależności od długości kolejki.
•	Logi do pliku:
Zamiast używać funkcji printf, zapis logów do pliku, co ułatwi analizę działania systemu.
•	Obsługa priorytetów:
Wprowadzenie priorytetów dla klientów (np. VIP), co umożliwiłoby obsługę najważniejszych klientów w pierwszej kolejności.
•	Optymalizacja opóźnień:
Dalsza optymalizacja opóźnień między przybyciem klientów i czasem strzyżenia w celu symulacji bardziej realistycznych scenariuszy.
•	Interfejs graficzny:
Rozbudowa interfejsu użytkownika o wizualizację stanu salonu, np. liczby klientów w poczekalni, aktualnego stanu kasy oraz obsługiwanych klientów.
________________________________________
Podsumowanie
Projekt "Symulacja Salonu Fryzjerskiego" został zaimplementowany z użyciem procesów POSIX, pamięci współdzielonej i semaforów. System umożliwia symulację różnych opcji płatności, w tym losowe wpłaty (30 zł, 50 zł, 100 zł) z odpowiednim wydawaniem reszty. Testy (mały salon, duży salon, zbyt wielu klientów, maksymalna liczba procesów) potwierdziły, że system działa zgodnie z oczekiwaniami, a wprowadzone mechanizmy synchronizacji zapewniają stabilność nawet przy bardzo dużej liczbie procesów. Projekt można łatwo skalować, zmieniając wartości w pliku nagłówkowym, co umożliwia symulację nawet ekstremalnych scenariuszy.

