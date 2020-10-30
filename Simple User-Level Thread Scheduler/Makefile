# .DEFAULT_GOAL=all
CC=gcc
CFLAGS=-fsanitize=signed-integer-overflow -fsanitize=undefined -g -std=gnu99 -O2 -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -lm

SERVER=backend

backend: a1_lib.c backend.c
	$(CC) -o $(SERVER) $(CFLAGS) backend.c a1_lib.c

sut: sut.c	
	$(CC) -o sut.o -c sut.c
	
clean:
	rm -rf sut.o $(SERVER)
