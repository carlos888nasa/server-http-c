CC = gcc 
CFLAGS = -Wall -Wextra -g -I./include
SRC = $(wildcard src/*.c)
TARGET = bin/server

all: clean $(TARGET)

$(TARGET):
	mkdir -p bin 
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean: 
	rm -f $(TARGET)  