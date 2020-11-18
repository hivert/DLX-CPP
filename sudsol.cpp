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

// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////
#include <cassert>
#include <chrono>
#include <cstring>
#include <ctime>  // time
#include <fstream>
#include <iomanip>
#include <iostream>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "dlx_matrix.hpp"
#include "hash_tuple.hpp"

namespace cron = std::chrono;

using item_t = std::tuple<char, int, int>;
using opt_t = std::tuple<int, int, int>;
using SQMatrix = std::vector<std::vector<int>>;
using ind_t = DLX_backtrack::DLXMatrix::ind_t;

int row_size, col_size, sq_size, nb_hint;
SQMatrix blocks, matrix;

std::vector<item_t> set_box_option(int row, int col, int nb) {
  std::vector<item_t> res;
  res.emplace_back('r', row, nb);
  res.emplace_back('c', col, nb);
  res.emplace_back('s', row, col);
  res.emplace_back('b', blocks[row - 1][col - 1], nb);
  return res;
}

void cout_mat(const SQMatrix &m) {
  for (int r = 0; r < sq_size; r++) {
    if ((row_size != 0) && (r % row_size == 0)) std::cout << "\n";
    std::cout << "  ";
    for (int c = 0; c < sq_size; c++) {
      if ((col_size != 0) && (c % col_size == 0)) std::cout << " ";
      if (m[r][c] != 0) {
        std::cout << m[r][c] << " ";
      } else {
        std::cout << ". ";
      }
    }
    std::cout << "\n";
  }
}

void read_sudoku(std::istream &in) {
  char type, unused;
  in >> type;
  switch (type) {
    case 's':
      in >> col_size >> unused >> row_size;
      assert(unused == 'x');
      sq_size = col_size * row_size;
      std::cout << "# Standard sudoku (block size = " << col_size << "x"
                << row_size << ", square size = " << sq_size << ")\n";
      break;

    case 'g':
      in >> sq_size;
      std::cout << "# Generalized sudoku " << sq_size << "x" << sq_size << "\n";
      break;

    default:
      std::cerr << "Unknown block type <" << type << std::endl;
      exit(EXIT_FAILURE);
  }

  // Dynamic allocation of the matrices.
  const std::vector<int> empty_row(sq_size);
  blocks.resize(sq_size, empty_row);
  matrix.resize(sq_size, empty_row);

  if (type == 's') {  // Standard block structure
    for (int r = 0; r < sq_size; r++) {
      for (int c = 0; c < sq_size; c++)
        blocks[r][c] = c / col_size + row_size * (r / row_size) + 1;
    }
  } else {  // Generalized block structure
    for (int r = 0; r < sq_size; r++)
      for (int c = 0; c < sq_size; c++) in >> blocks[r][c];
  }

  // Hint of the problem statement
  nb_hint = 0;
  for (int r = 0; r < sq_size; r++) {
    for (int c = 0; c < sq_size; c++) {
      char ch = in.peek();
      while (ch == ' ' || ch == '\n') {
        in.ignore();
        ch = in.peek();
      }
      matrix[r][c] = 0;
      if (ch == '.') {
        in.ignore();
      } else {
        matrix[r][c] = 0;
        in >> matrix[r][c];
        if (matrix[r][c] == 0) {
          std::cerr << "Bad character <" << in.peek() << ">" << std::endl;
          exit(EXIT_FAILURE);
        } else {
          nb_hint++;
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  auto tstart = cron::high_resolution_clock::now();
  if (argc > 1) {
    std::ifstream ifile(argv[1]);
    if (ifile) {
      read_sudoku(ifile);
    } else {
      std::cerr << "File not found : " << argv[1] << std::endl;
      exit(EXIT_FAILURE);
    }
  } else {
    read_sudoku(std::cin);
  }

  auto tencode = std::chrono::high_resolution_clock::now();
  std::vector<item_t> items;
  for (int i = 1; i <= sq_size; i++)  // Square i,j occupied
    for (int j = 1; j <= sq_size; j++) items.emplace_back('s', i, j);
  for (int i = 1; i <= sq_size; i++)  // Block i occupied by j
    for (int j = 1; j <= sq_size; j++) items.emplace_back('b', i, j);
  for (int i = 1; i <= sq_size; i++)  // Row i occupied by j
    for (int j = 1; j <= sq_size; j++) items.emplace_back('r', i, j);
  for (int i = 1; i <= sq_size; i++)  // Col i occupied by j
    for (int j = 1; j <= sq_size; j++) items.emplace_back('c', i, j);

  DLX_backtrack::DLXMatrixIdent<item_t, opt_t, hash_tuple::hash<item_t>> M(
      std::move(items));  // items is no more needed

  // Rules of the Sudoku game
  for (int r = 1; r <= sq_size; r++) {
    for (int c = 1; c <= sq_size; c++) {
      for (int n = 1; n <= sq_size; n++) {
        M.add_opt({r, c, n}, set_box_option(r, c, n));
      }
    }
  }
  for (int r = 1; r <= sq_size; r++) {
    for (int c = 1; c <= sq_size; c++) {
      if (matrix[r - 1][c - 1] != 0) {
        M.choose({r, c, matrix[r - 1][c - 1]});
      }
    }
  }

  auto tcompute = std::chrono::high_resolution_clock::now();

  if (!M.search_iter()) {
    std::cout << "No solution found !" << std::endl;
    exit(EXIT_FAILURE);
  }
  auto soldance = M.get_solution();
  if (M.search_iter()) {
    std::cout << "More than one solution found !" << std::endl;
    exit(EXIT_FAILURE);
  }
  SQMatrix solution(sq_size, std::vector<int>(sq_size));
  for (auto [r, c, n] : soldance) solution[r - 1][c - 1] = n;

  auto endcompute = cron::high_resolution_clock::now();
  std::cout << std::endl;
  cout_mat(solution);
  std::cout << std::endl;
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
