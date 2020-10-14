SHELL = /bin/sh
CFLAGS = -Wall -lrt $(DEBUG)
CXXFLAGS= -Wall -std=c++17 -g -O3
CC = gcc

MAIN_FILES = test_dlx_matrix dlx_matrix_test sudsol

#### Dépendances ####
.PHONY: clean all
all: $(MAIN_FILES)

dlx_matrix.o: dlx_matrix.cpp dlx_matrix.hpp

dlx_matrix_test: CXXFLAGS += -DDOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
dlx_matrix_test: dlx_matrix.cpp dlx_matrix.hpp
	${CXX} ${CXXFLAGS} dlx_matrix.cpp -o dlx_matrix_test

test_dlx_matrix: dlx_matrix.o
sudsol: dlx_matrix.o

#### Cibles diverses ####
clean:
	$(RM) *.o $(MAIN_FILES)

check: dlx_matrix_test
	./dlx_matrix_test
