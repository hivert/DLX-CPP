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
#ifndef DLX_MATRIX_HPP_
#define DLX_MATRIX_HPP_

#include <climits>
#include <iostream>
#include <vector>

namespace DLX_backtrack {

struct size_mismatch_error : public std::runtime_error {
    size_mismatch_error(const std::string &s, size_t expected, size_t sz)
        : std::runtime_error("Wrong " + s + " size : " + std::to_string(sz) +
                             " (expecting " + std::to_string(expected) + ")") {}
};

struct out_of_bound_error : public std::runtime_error {
    out_of_bound_error(const std::string &s, size_t bound, size_t i)
        : std::runtime_error("Value " + s + " too large : " + std::to_string(i) +
                             " (at most " + std::to_string(bound) + ")") {}
};

void check_size(const std::string &s, size_t expected, size_t sz);
void check_bound(const std::string &s, size_t bound, size_t i);

std::vector<int> inverse_perm(const std::vector<int> &perm);

/////////////////
class DLXMatrix {
  private:

    struct Header;
    struct Node {
        size_t row_id;
        Node *left, *right, *up, *down;
        Header *head;
    };

    struct Header {
        size_t col_id, size;
        Node node;
        Header *left, *right;
    };

    size_t nb_primary;
    std::vector<Header> heads;
    std::vector<std::vector<Node>> rows;

    std::vector<Node *> work;
    bool search_down;

  public:

    DLXMatrix() = delete;
    explicit DLXMatrix(size_t nb_col) : DLXMatrix(nb_col, nb_col) {}
    DLXMatrix(size_t nb_col, size_t nb_prim);
    DLXMatrix(size_t nb_col, const std::vector<std::vector<int>> &rows)
        : DLXMatrix(nb_col, nb_col, rows) {}
    DLXMatrix(size_t nb_col, size_t nb_prim,
              const std::vector<std::vector<int>> &);
    DLXMatrix(const DLXMatrix &);
    DLXMatrix &operator=(DLXMatrix other);

    size_t width() const { return heads.size() - 1; }
    size_t height() const { return rows.size(); }

    void check_sizes() const;

    int add_row(const std::vector<int> &r) { return add_row_sparse(r); }
    int add_row_sparse(const std::vector<int> &r);
    int add_row_dense(const std::vector<bool> &r);
    std::vector<int> row_sparse(size_t i) const;
    std::vector<bool> row_dense(size_t i) const;

    std::vector<int> row_to_sparse(const std::vector<bool> &row) const;
    std::vector<bool> row_to_dense(std::vector<int> row) const;

    std::vector<std::vector<int>> search_rec(size_t max_sol = SIZE_MAX);
    bool search_iter();
    bool search_iter(std::vector<int> &);
    std::vector<int> get_solution();
    bool search_random(std::vector<int> &);

    bool is_solution(const std::vector<int> &);

    void reset();

    DLXMatrix permuted_columns(const std::vector<int> &perm);
    DLXMatrix permuted_inv_columns(const std::vector<int> &perm);
    DLXMatrix permuted_rows(const std::vector<int> &perm);

    friend std::ostream &operator<<(std::ostream &, const DLXMatrix &);

    int nb_choices, nb_dances;  // Computation statistics

  protected:

    Header *master() { return &heads[0]; }
    const Header *master() const { return &heads[0]; }

    Header *choose_min();
    void cover(Header *col);
    void uncover(Header *col);
    void choose(Node *row);
    void unchoose(Node *row);
    void search_rec_internal(size_t, std::vector<std::vector<int>> &);

    static std::vector<int> row_sparse(const std::vector<Node> &);
    std::vector<bool> row_dense(const std::vector<Node> &) const;
    void print_solution(const std::vector<Node *> &) const;
};


template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v);

}  // namespace DLX_backtrack

#endif  // DLX_MATRIX_HPP_
