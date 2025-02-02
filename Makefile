CC = gcc
CFLAGS = -Wall -std=c99 -pthread

# Lista plików źródłowych
SOURCES = main.c fryzjer.c klient.c kierownik.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = salon-fryzjerski

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lrt

%.o: %.c wspolne.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(OBJECTS) $(TARGET)

.PHONY: all clean
