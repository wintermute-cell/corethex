CC=gcc
CFLAGS=-I./src -lncurses

hellomake: src/main.c
	$(CC) $(CFLAGS) -o bin/corethex src/main.c
