CC = gcc
CFLAGS = -Wall -pthread

OBJS = main.o fryzjer.o klient.o

salon: $(OBJS)
	$(CC) $(CFLAGS) -o salon $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o salon
