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
#include <sstream>        // std::ostringstream
#include <cassert>
#include <ctime>          // time
#include <vector>
#include <tuple>
#include <unordered_map>
#include "dance.hpp"



int row_size, col_size, sq_size;
std::vector<std::string> col_names;
std::vector<std::tuple<int, int, int>> row_names;
std::unordered_map<std::string, size_t> col_ranks;


void new_col(std::string name) {
    col_ranks.insert(std::make_pair(name, col_names.size()));
    col_names.push_back(name);
}

std::string col_name(char h, int i, int j) {
    std::ostringstream col_name;
    col_name << h << "_" << i << "_" << j;
    return col_name.str();
}

std::vector<int> row_case_occ(
    int row, int col, int number, int sq_size, void *bl) {
    int (*block)[sq_size] = static_cast<int (*)[sq_size]>(bl);
    std::vector<int> res;
    res.push_back(col_ranks[col_name('r', row, number)]);
    res.push_back(col_ranks[col_name('c', col, number)]);
    res.push_back(col_ranks[col_name('s', row, col)]);
    res.push_back(col_ranks[col_name('b', block[row-1][col-1], number)]);
    return res;
}

std::string hint_name(int i) {
    std::ostringstream col_name;
    col_name << "en_" << std::setfill('0') << std::setw(3) << i;
    return col_name.str();
}

int main(void) {
    char type, unused;

    std::cin >> type;
    switch (type) {
    case 's' :
        std::cin >> col_size >> unused >> row_size;
        assert (unused == 'x');
	sq_size = col_size*row_size;
        std::cout << "%S s" << col_size << "x" << row_size << "\n";
        std::cout << "%C Standard sudoku (block size = "
                  << col_size << "x" << row_size
                  << ", square size = " << sq_size << ")\n";
        break;
    case 'g' :
        std::cin >> sq_size;
        std::cout << "%S g" << sq_size << "\n";
 	std::cout << "%C Generalized sudoku "
                  << sq_size << "x" << sq_size << "\n";
        break;
    default :
        std::cerr << "Unknown block type <" << type << std::endl;
	exit(EXIT_FAILURE);
    }

    // Dynamic allocation of the matrices.
    int block[sq_size][sq_size];
    int matrix[sq_size][sq_size];

    if (type=='s') // Standard block structure
        for (int i = 0; i < sq_size; i++)
            for (int j = 0; j < sq_size; j++)
                block[i][j] = j / col_size + row_size*(i / row_size)+1;
    else // Generalized standard block structure
        for (int i = 0; i < sq_size; i++)
            for (int j = 0; j < sq_size; j++)
                std::cin >> block[i][j];

    // for (int i = 0; i < sq_size; i++) {
    //     for (int j = 0; j < sq_size; j++)
    //         std::cout << block[i][j] << " ";
    //     std::cout << "\n";
    // }

    // Hint of the enonce of the problem
    int n_hint = 0;
    for (int i = 0; i < sq_size; i++) {
        for (int j = 0; j < sq_size; j++) {
            char c = std::cin.peek();
            while (c == ' ' or c == '\n') {
                std::cin.get();
                c = std::cin.peek();
            }
            matrix[i][j] = 0;
            if (c == '.') {
                std::cin.get();
            } else {
                matrix[i][j] = 0;
                std::cin >> matrix[i][j];
                if (matrix[i][j] == 0) {
                    std::cerr << "Bad character <" << c << ">" << std::endl;
                    exit(EXIT_FAILURE);
                }
                else {
                    n_hint++;
                }
            }
        }
    }

    // for (int i = 0; i < sq_size; i++) {
    //     for (int j = 0; j < sq_size; j++)
    //         std::cout << matrix[i][j] << " ";
    //     std::cout << "\n";
    // }

    for (int i = 1; i <= sq_size; i++)
        for (int j = 1; j <= sq_size; j++)
            new_col(col_name('s', i, j));  // Square i,j occupied
    for (int i = 1; i <= sq_size; i++)
        for (int j = 1; j <= sq_size; j++)
            new_col(col_name('b', i, j));  // Block i occupied by j
    for (int i = 1; i <= sq_size; i++)
        for (int j = 1; j <= sq_size; j++)
            new_col(col_name('r', i, j));  // Row i occupied by j
    for (int i = 1; i <= sq_size; i++)
        for (int j = 1; j <= sq_size; j++)
            new_col(col_name('c', i, j));  // Col i occupied by j
    for (int i = 0; i < n_hint; i++)
        new_col(hint_name(i));

    // for (auto s : col_names) std::cout << s << " ";
    // std::cout << std::endl;
    // for (auto s : col_names) std::cout << col_ranks[s] << " ";
    // std::cout << std::endl;

    DLXMatrix M(col_names.size());
    // Rule of the Sudoku game
    for (int r = 1; r <= sq_size; r++)
        for (int c = 1; c <= sq_size; c++)
            for (int n = 1; n <= sq_size; n++) {
                M.add_row(row_case_occ(r, c, n, sq_size, block));
                row_names.push_back({r, c, n});
            }

    n_hint =0;
    for (int r = 1; r <= sq_size; r++)
        for (int c = 1; c <= sq_size; c++)
            if (matrix[r-1][c-1] != 0) {
                auto row = row_case_occ(r, c, matrix[r-1][c-1], sq_size, block);
                row.push_back(col_ranks[hint_name(n_hint)]);
                M.add_row(row);
                row_names.push_back({r, c, matrix[r-1][c-1]});
                n_hint++;
            }
    auto res = M.search_rec(2);
    assert(res.size() == 1);
    int solution[sq_size][sq_size];
    for (int rind : res[0]) {
        auto [r, c, n] = row_names[rind];
//        std::cout << r << " " << c << " " << n << std::endl;
        solution[r-1][c-1] = n;
    }
    for (int i = 0; i < sq_size; i++) {
        for (int j = 0; j < sq_size; j++)
            std::cout << solution[i][j] << " ";
        std::cout << "\n";
    }

}
