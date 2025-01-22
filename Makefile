CC = gcc
CFLAGS = -Wall -pthread
OBJ = main.o klient.o

all: salon

salon: $(OBJ)
	$(CC) $(CFLAGS) -o salon $(OBJ)

main.o: main.c klient.h
	$(CC) $(CFLAGS) -c main.c

klient.o: klient.c klient.h
	$(CC) $(CFLAGS) -c klient.c

clean:
	rm -f salon $(OBJ)
