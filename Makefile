# COMP90077 - Assignment 2 - James Barnes (820946)
# Makefile for testing program

CC     = gcc
CFLAGS = -Wall -std=c99 -O0
EXE    = prog
OBJ    = main.o data.o rtree.o

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)

main.o: main.h data.h rtree.h
data.o: data.h main.h
rtree.o: rtree.h main.h data.h

.PHONY: clean cleanly all CLEAN

clean:
	rm -f $(OBJ)
CLEAN: clean
	rm -f $(EXE)
cleanly: all clean
