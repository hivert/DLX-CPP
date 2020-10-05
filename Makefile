SHELL = /bin/sh
CFLAGS = -Wall -lrt $(DEBUG)
CPPFLAGS= -Wall -std=c++17 -g # -O3
CC = gcc

MAIN_FILES = dancing sudoku2dance sol2sudoku sol2mupad dance

#### Pour que les .c dépendent des .h ####
%.o : %.c %.h
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

dance: dance.cpp dance.hpp


#### Dépendances ####
.PHONY: clean all
all: $(MAIN_FILES)

#### Cibles diverses ####
clean:
	$(RM) *.o $(MAIN_FILES)

check: dancing
	cat example.txt | ./dancing
checkSudoku: all
	cat examples/sudoku2.txt | ./sudoku2dance | ./dancing | ./sol2sudoku 
