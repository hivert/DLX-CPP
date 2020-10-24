SHELL = /bin/sh
CFLAGS = -Wall -lrt $(DEBUG)
CXXFLAGS= -Wall -std=c++17 -g -O3
CC = gcc

MAIN_FILES = sudsol dlx_matrix_test block_diagram_test

#### Dépendances ####
.PHONY: clean all
all: $(MAIN_FILES)

dlx_matrix.o: dlx_matrix.cpp dlx_matrix.hpp

libdlx_matrix.o: CXXFLAGS += -fPIC
libdlx_matrix.o: libdlx_matrix.cpp dlx_matrix.hpp

libdlx_matrix.so: CXXFLAGS += -fPIC
libdlx_matrix.so: libdlx_matrix.o
	$(LINK.c) -shared $^ -o $@

dlx_matrix_test: CXXFLAGS += -DDOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
dlx_matrix_test: dlx_matrix.cpp dlx_matrix.hpp
	${CXX} ${CXXFLAGS} dlx_matrix.cpp -o dlx_matrix_test

block_diagram_test: CXXFLAGS += -DDOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
block_diagram_test: block_diagram.cpp block_diagram.hpp
	${CXX} ${CXXFLAGS} block_diagram.cpp -o block_diagram_test

sudsol: dlx_matrix.o

#### Cibles diverses ####
clean:
	$(RM) *.o *.so $(MAIN_FILES)

check-dlx_matrix: dlx_matrix_test
	./dlx_matrix_test
check-block_diagram: block_diagram_test
	./block_diagram_test
check-sudsol: sudsol
	@echo -n "Testing sudsol : "; \
	   ./sudsol examples/sudoku1.txt | grep -v '^# ' | \
	   diff - sudoku1.output.txt; \
	   if [ $$? -eq 0 ]; then echo "PASS"; else echo "FAIL"; fi
check-inter: libdlx_matrix.so
	sage -t inter.sage

check: check-dlx_matrix check-block_diagram check-sudsol check-inter
