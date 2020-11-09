//****************************************************************************//
//       Copyright (C) 2020 Florent Hivert <Florent.Hivert@lri.fr>,           //
//                                                                            //
//    Distributed under the terms of the GNU General Public License (GPL)     //
//                                                                            //
//    This code is distributed in the hope that it will be useful,            //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of          //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       //
//    General Public License for more details.                                //
//                                                                            //
//    The full text of the GPL is available at:                               //
//                                                                            //
//                  http://www.gnu.org/licenses/                              //
//****************************************************************************//

// Solution of Langford pair problem using DLX
//////////////////////////////////////////////
#include <cassert>
#include <chrono>
#include <cstring>
#include <ctime>  // time
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "dlx_matrix.hpp"
#include "utils.hpp"

namespace cron = std::chrono;

char hex(size_t l) { return l < 10 ? '0' + l : 'a' + l - 10; }

std::string sol_to_string(size_t N, DLX_backtrack::DLXMatrix &M,
                          DLX_backtrack::DLXMatrix::Vect1D sol) {
  std::string res(2 * N, '_');
  for (size_t irow : sol) {
    auto row = M.row_sparse(irow);
    size_t l = row[0] + 1;
    size_t pos1 = row[1] - N;
    res[pos1] = hex(l);
    res[pos1 + l + 1] = hex(l);
  }
  return res;
}

int main(int argc, char *argv[]) {
  size_t N = 4;
  auto tstart = cron::high_resolution_clock::now();
  if (argc == 2) {
    char *check;
    N = strtol(argv[1], &check, 10);
    if (*check != '\0') {
      std::cerr << "bad argument: " << argv[1] << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  if (argc > 2) {
    std::cerr << "Too many argument : " << argc << std::endl;
    exit(EXIT_FAILURE);
  }

  auto tencode = std::chrono::high_resolution_clock::now();
  DLX_backtrack::DLXMatrix M(3 * N);

  // ‘i sj sk’, for 1 ≤ j < k ≤ 2n, k = i + j + 1, 1 ≤ i ≤ n;

  // Columns:
  // - 0..n-1 : letter l+1 used
  // - n .. 3n - 1 : place i + 1 - n used

  for (size_t i = 1; i <= N; i++) {
    for (size_t pos = 1; pos + i + 1 <= 2 * N; pos++) {
      M.add_row({i - 1, N + pos - 1, N + pos + i});
    }
  }
  // std::cout << M << std::endl;

  auto tcompute = std::chrono::high_resolution_clock::now();
  std::vector<size_t> soldance;
  if (!M.search_iter(soldance)) {
    std::cout << "No solution found !" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << sol_to_string(N, M, soldance) << std::endl;
  // search for other solutions
  size_t nsol = 1;
  while (M.search_iter()) nsol++;
  auto endcompute = cron::high_resolution_clock::now();

  std::cout << "Number of solutions: " << nsol << std::endl;
  auto endprint = cron::high_resolution_clock::now();
  std::cout << "# Number of choices: " << M.nb_choices
            << ", Number of dances: " << M.nb_dances << "\n";
  std::cout << std::fixed << std::setprecision(0) << "# Timings: parse = "
            << cron::duration<float, std::micro>(tencode - tstart).count()
            << "μs, encode = "
            << cron::duration<float, std::micro>(tcompute - tencode).count()
            << "μs, solve = "
            << cron::duration<float, std::micro>(endcompute - tcompute).count()
            << "μs, output = "
            << cron::duration<float, std::micro>(endprint - endcompute).count()
            << "μs\n# Total = "
            << cron::duration<float, std::micro>(endprint - tstart).count()
            << "μs\n";
}
