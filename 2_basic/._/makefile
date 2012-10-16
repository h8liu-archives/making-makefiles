.PHONY: all clean run

CC := gcc
CFLAGS := -Wall -O3
TARGET := hello

all: $(TARGET)

$(TARGET): hello.o main.o
	$(CC) $(CFLAGS) hello.o main.o -o hello

hello.o: hello.c hello.h
	$(CC) $(CFLAGS) -c hello.c -o hello.o

main.o: main.c hello.h
	$(CC) $(CFLAGS) -c main.c -o main.o

clean: ; -rm -f *.o $(TARGET)
run: ;./$(TARGET)
