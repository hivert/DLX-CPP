SHELL = /bin/sh
CFLAGS = -Wall -lrt $(DEBUG)
CXXFLAGS= -Wall -std=c++17 -g -O3
CC = gcc

MAIN_FILES = test_dance sudsol

#### Dépendances ####
.PHONY: clean all
all: $(MAIN_FILES)

dance.o: dance.cpp dance.hpp
test_dance: dance.o
sudsol: dance.o

#### Cibles diverses ####
clean:
	$(RM) *.o $(MAIN_FILES)

check: test_dance
	./test_dance
