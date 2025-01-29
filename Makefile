CC = gcc
CFLAGS = -Wall -pthread
OBJ = main.o fryzjer.o klient.o

all: salon

salon: $(OBJ)
	$(CC) $(CFLAGS) -o salon $(OBJ)

%.o: %.c salon.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o salon
