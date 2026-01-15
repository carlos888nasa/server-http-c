CC = gcc 
CFLAGS = -Wall -Wextra -g -I./include
SRC = src/server.c src/main.c
TARGET = bin/server

all: clean $(TARGET)

$(TARGET):
	mkdir -p bin 
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean: 
	rm -f $(TARGET)  