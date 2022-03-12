CC=gcc
CFLAGS=-I./src -lncurses

hellomake: src/main.c
	$(CC) $(CFLAGS) -o corethex src/main.c
