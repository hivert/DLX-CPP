SHELL = /bin/sh
CFLAGS = -Wall -lrt $(DEBUG)
CXXFLAGS= -Wall -std=c++17 -g -O3
CC = gcc

MAIN_FILES = test_dance dance_test sudsol

#### Dépendances ####
.PHONY: clean all
all: $(MAIN_FILES)

dance.o: dance.cpp dance.hpp

dance_test: CXXFLAGS += -DDOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
dance_test: dance.cpp dance.hpp
	${CXX} ${CXXFLAGS} dance.cpp -o dance_test

test_dance: dance.o
sudsol: dance.o

#### Cibles diverses ####
clean:
	$(RM) *.o $(MAIN_FILES)

check: dance_test
	./dance_test
