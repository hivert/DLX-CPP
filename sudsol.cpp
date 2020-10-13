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
#include <ctime>          // time
#include <vector>
#include <tuple>
#include <unordered_map>
#include "dance.hpp"


namespace std {
    namespace {

        // Code from boost
        // Reciprocal of the golden ratio helps spread entropy
        //     and handles duplicates.
        // See Mike Seymour in magic-numbers-in-boosthash-combine:
        //     https://stackoverflow.com/questions/4948780

        template <class T>
        inline void hash_combine(std::size_t &seed, T const &v) {
            seed ^= hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        }

        template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
        struct HashValueImpl {
            static void apply(size_t &seed, Tuple const &tuple) {
                HashValueImpl<Tuple, Index-1>::apply(seed, tuple);
                hash_combine(seed, get<Index>(tuple));
            }
        };

        template <class Tuple>
        struct HashValueImpl<Tuple, 0> {
            static void apply(size_t &seed, Tuple const &tuple) {
                hash_combine(seed, get<0>(tuple));
            }
        };
    }

    template <typename ... TT>
    struct hash<std::tuple<TT...> > {
        size_t operator()(std::tuple<TT...> const &tt) const {
            size_t seed = 0;
            HashValueImpl<std::tuple<TT...> >::apply(seed, tt);
            return seed;
        }
    };
}


using col_type = std::tuple<char, int, int>;
using SQMatrix = std::vector<std::vector<int> >;

int row_size, col_size, sq_size;
std::vector<col_type> col_names;
std::vector<std::tuple<int, int, int> > row_codes;
std::unordered_map<col_type, size_t> col_ranks;


void new_col(col_type name) {
    col_ranks.insert(std::make_pair(name, col_names.size()));
    col_names.push_back(name);
}

std::vector<int> row_case_occ(int row, int col, int nb, SQMatrix &blocks) {
    std::vector<int> res;
    res.push_back(col_ranks[{'r', row, nb}]);
    res.push_back(col_ranks[{'c', col, nb}]);
    res.push_back(col_ranks[{'s', row, col}]);
    res.push_back(col_ranks[{'b', blocks[row-1][col-1], nb}]);
    return res;
}


void cout_mat(SQMatrix &m) {
    for (int r = 0; r < sq_size; r++) {
        if ((row_size != 0) && !(r % row_size)) std::cout << "\n";
        std::cout << "  ";
        for (int c = 0; c < sq_size; c++) {
            if ((col_size != 0) && !(c % col_size)) std::cout << " ";
            if (m[r][c] != 0)  std::cout << m[r][c] << " ";
            else               std::cout << ". ";
        }
        std::cout << "\n";
    }
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
    const std::vector<int> empty_row(sq_size);
    SQMatrix blocks(sq_size, empty_row);
    SQMatrix matrix(sq_size, empty_row);

    if (type=='s') {  // Standard block structure
        for (int r = 0; r < sq_size; r++)
            for (int c = 0; c < sq_size; c++)
                blocks[r][c] = c / col_size + row_size*(r / row_size)+1;
    } else {  // Generalized block structure
        for (int r = 0; r < sq_size; r++)
            for (int c = 0; c < sq_size; c++)
                std::cin >> blocks[r][c];
    }

    // Hint of the problem statement
    int n_hint = 0;
    for (int r = 0; r < sq_size; r++) {
        for (int c = 0; c < sq_size; c++) {
            char ch = std::cin.peek();
            while (ch == ' ' or ch == '\n') {
                std::cin.get();
                ch = std::cin.peek();
            }
            matrix[r][c] = 0;
            if (ch == '.') {
                std::cin.get();
            } else {
                matrix[r][c] = 0;
                std::cin >> matrix[r][c];
                if (matrix[r][c] == 0) {
                    std::cerr << "Bad character <" << ch << ">" << std::endl;
                    exit(EXIT_FAILURE);
                }
                else {
                    n_hint++;
                }
            }
        }
    }

    cout_mat(matrix);

    for (int i = 1; i <= sq_size; i++)
        for (int j = 1; j <= sq_size; j++)
            new_col({'s', i, j});  // Square i,j occupied
    for (int i = 1; i <= sq_size; i++)
        for (int j = 1; j <= sq_size; j++)
            new_col({'b', i, j});  // Block i occupied by j
    for (int i = 1; i <= sq_size; i++)
        for (int j = 1; j <= sq_size; j++)
            new_col({'r', i, j});  // Row i occupied by j
    for (int i = 1; i <= sq_size; i++)
        for (int j = 1; j <= sq_size; j++)
            new_col({'c', i, j});  // Col i occupied by j
    for (int i = 0; i < n_hint; i++)
        new_col({'e', i, 0});

    DLXMatrix M(col_names.size());

    // Rule of the Sudoku game
    for (int r = 1; r <= sq_size; r++) {
        for (int c = 1; c <= sq_size; c++) {
            for (int n = 1; n <= sq_size; n++) {
                M.add_row(row_case_occ(r, c, n, blocks));
                row_codes.push_back({r, c, n});
            }
        }
    }
    n_hint =0;
    for (int r = 1; r <= sq_size; r++) {
        for (int c = 1; c <= sq_size; c++) {
            if (matrix[r-1][c-1] != 0) {
                auto row = row_case_occ(r, c, matrix[r-1][c-1], blocks);
                row.push_back(col_ranks[{'e', n_hint, 0}]);
                M.add_row(row);
                row_codes.push_back({r, c, matrix[r-1][c-1]});
                n_hint++;
            }
        }
    }
    auto res = M.search_rec(2);
    assert(res.size() == 1);

    SQMatrix solution(sq_size, empty_row);

    for (int rind : res[0]) {
        auto [r, c, n] = row_codes[rind];
        solution[r-1][c-1] = n;
    }
    std::cout << std::endl;
    cout_mat(solution);
}
