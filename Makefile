SHELL = /bin/sh
CFLAGS = -Wall -lrt $(DEBUG)
CPPFLAGS= -Wall -std=c++17 -g -O3
CC = gcc

MAIN_FILES = dancemain

#### Dépendances ####
.PHONY: clean all
all: $(MAIN_FILES)

dance.o: dance.cpp dance.hpp
dancemain: dance.o

#### Cibles diverses ####
clean:
	$(RM) *.o $(MAIN_FILES)

check: dancing
	cat example.txt | ./dancing
checkSudoku: all
	cat examples/sudoku2.txt | ./sudoku2dance | ./dancing | ./sol2sudoku 
