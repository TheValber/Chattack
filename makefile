CC=clang
CFLAGS=-std=c17 -Wall -Wfatal-errors

EXE=chattack

all:
	$(CC) $(CFLAGS) main.c -o $(EXE) -lMLV
