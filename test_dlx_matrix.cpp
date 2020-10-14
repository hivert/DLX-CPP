//****************************************************************************//
//       Copyright (C) 2020 Florent Hivert <Florent.Hivert@lri.fr>,           //
//                                                                            //
//    Distributed under the terms of the GNU General Public License (GPL)     //
//                                                                            //
//    This code is distributed in the hope that it will be useful,            //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of          //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       //
//   General Public License for more details.                                 //
//                                                                            //
//  The full text of the GPL is available at:                                 //
//                                                                            //
//                  http://www.gnu.org/licenses/                              //
//****************************************************************************//

// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#include <cstring>
#include <iostream>
#include <cassert>
#include <ctime>      // For time
#include <algorithm>  // For shuffle
#include <vector>
#include "dlx_matrix.hpp"



int main(int argc, char** argv) {
    doctest::Context ctx;
    // default - stop after 5 failed asserts
    ctx.setOption("abort-after", 5);
    // apply command line - argc / argv
    ctx.applyCommandLine(argc, argv);
    // override - don't break in the debugger
    ctx.setOption("no-breaks", true); 
    // run test cases unless with --no-run
    int res = ctx.run();
    // query flags (and --exit) rely on this
    if(ctx.shouldExit())
        // propagate the result of the tests
        return res;





    std::srand(static_cast<unsigned>(std::time(0)));

    DLX_backtrack::DLXMatrix M(6);
    std::vector<int> vctr{3, 2, 1, 4, 0}, vctc{3, 2, 1, 4, 5, 0};

    M.add_row({0, 2});
    M.add_row({0, 1});
    M.add_row({1, 4});
    M.add_row({3});
    M.add_row({3, 4});

    std::cout << M << "\n" << vctr << "\n\n" << M.permuted_rows(vctr)
              << "\n" << vctc << "\n\n" << M.permuted_columns(vctc) << "\n";
    M.add_row({5});
    M.add_row({1});
    M.add_row({0, 1, 2});
    M.add_row({2, 3, 4});
    M.add_row({1, 4, 5});
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
    DLX_backtrack::DLXMatrix N(M);
    N.search_iter(); std::cout << N.get_solution() << std::endl;
    N.search_iter(); std::cout << N.get_solution() << std::endl;
    N.search_iter(); std::cout << N.get_solution() << std::endl;
    std::cout << "Assign ========================= \n";


    N = M;
    N.reset();
    std::vector<int> sol;
    while (N.search_iter(sol)) std::cout << sol << std::endl;

    std::cout << "Random ========================= \n";
    std::vector<int> rand;
    std::cout << N.search_random(rand) << " " << rand << std::endl;

    return res;
}

