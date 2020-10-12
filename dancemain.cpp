// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////
#include <cstring>
#include <iostream>
#include <algorithm>
#include <cassert>
#include "dance.hpp"



int main() {
    DLXMatrix M(6);
    std::vector<int> vctr{3,2,1,4,0}, vctc{3,2,1,4,5,0};

    M.add_row({0,2});
    M.add_row({0,1});
    M.add_row({1,4});
    M.add_row({3});
    M.add_row({3,4});

    std::cout << M << "\n" << vctr << "\n\n" << M.permuted_rows(vctr)
              << "\n" << vctc << "\n\n" << M.permuted_columns(vctc) << "\n";
    M.add_row({5});
    M.add_row({1});
    M.add_row({0,1,2});
    M.add_row({2,3,4});
    M.add_row({1,4,5});
    M.check_sizes();

    const int maxsol = 3;
    std::cout << "Recursive ========================= \n";
    for (auto &s : M.search_rec(maxsol))
        std::cout << s << std::endl;
    std::cout << "Iterative ========================= \n";
    M.reset();
    for (int i=0; i < 3; i++) {
        M.search_iter(); std::cout << M.get_solution() << std::endl;
    }
    std::cout << "Reset ========================= \n";
    M.reset();
    M.search_iter(); std::cout << M.get_solution() << std::endl;
    M.search_iter(); std::cout << M.get_solution() << std::endl;
    std::cout << "Copy ========================= \n";
    DLXMatrix N(M);
    N.search_iter(); std::cout << N.get_solution() << std::endl;
    N.search_iter(); std::cout << N.get_solution() << std::endl;
    N.search_iter(); std::cout << N.get_solution() << std::endl;
    std::cout << "Assign ========================= \n";
    N = M;
    N.search_iter(); std::cout << N.get_solution() << std::endl;
    N.search_iter(); std::cout << N.get_solution() << std::endl;
    N.search_iter(); std::cout << N.get_solution() << std::endl;
}

