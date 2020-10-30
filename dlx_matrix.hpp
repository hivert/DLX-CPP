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
#include <vector>    // vector
#include <memory>    // unique_ptr

namespace DLX_backtrack {

struct size_mismatch_error : public std::runtime_error {
    size_mismatch_error(const std::string &s, size_t expected, size_t sz)
        : std::runtime_error("Wrong " + s + " size : " + std::to_string(sz) +
                             " (expecting " + std::to_string(expected) + ")") {}
};

struct out_of_bound_error : public std::runtime_error {
    out_of_bound_error(const std::string &s, size_t bound, size_t i)
        : std::runtime_error("Value " + s +
                             " too large : " + std::to_string(i) +
                             " (at most " + std::to_string(bound) + ")") {}
};

void check_size(const std::string &s, size_t expected, size_t sz);
void check_bound(const std::string &s, size_t bound, size_t i);

std::vector<size_t> inverse_perm(const std::vector<size_t> &perm);

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

    std::unique_ptr<Header> master;
    std::vector<Header> heads;
    size_t nb_primary;

    std::vector<std::vector<Node>> rows;

    std::vector<Node *> work;
    bool search_down;

  public:
    using Vect1D = std::vector<size_t>;
    using Vect2D = std::vector<Vect1D>;

    DLXMatrix() : DLXMatrix(0) {};
    explicit DLXMatrix(size_t nb_col) : DLXMatrix(nb_col, nb_col) {}
    DLXMatrix(size_t nb_col, size_t nb_prim);
    DLXMatrix(size_t nb_col, const Vect2D &rows)
        : DLXMatrix(nb_col, nb_col, rows) {}
    DLXMatrix(size_t nb_col, size_t nb_prim, const Vect2D &rows);
    DLXMatrix(const DLXMatrix &);
    DLXMatrix &operator=(const DLXMatrix &other);
    DLXMatrix(DLXMatrix &&) = default;
    DLXMatrix &operator=(DLXMatrix &&other) = default;
    ~DLXMatrix() = default;

    size_t width() const { return heads.size(); }
    size_t height() const { return rows.size(); }

    void check_sizes() const;

    size_t add_row(const Vect1D &r) { return add_row_sparse(r); }
    size_t add_row_sparse(const Vect1D &r);
    size_t add_row_dense(const std::vector<bool> &r);
    Vect1D row_sparse(size_t i) const;
    std::vector<bool> row_dense(size_t i) const;

    Vect1D row_to_sparse(const std::vector<bool> &row) const;
    std::vector<bool> row_to_dense(Vect1D row) const;

    Vect2D search_rec(size_t max_sol = SIZE_MAX);
    bool search_iter();
    bool search_iter(Vect1D &);
    Vect1D get_solution();
    bool search_random(Vect1D &);

    bool is_solution(const Vect1D &);

    void reset();

    DLXMatrix permuted_columns(const Vect1D &perm);
    DLXMatrix permuted_inv_columns(const Vect1D &perm);
    DLXMatrix permuted_rows(const Vect1D &perm);

    friend std::ostream &operator<<(std::ostream &, const DLXMatrix &);

    size_t nb_choices, nb_dances;  // Computation statistics

  protected:

    Header *choose_min();
    void cover(Header *col);
    void uncover(Header *col);
    void choose(Node *row);
    void unchoose(Node *row);
    void search_rec_internal(size_t, Vect2D &);

    static Vect1D row_sparse(const std::vector<Node> &);
    std::vector<bool> row_dense(const std::vector<Node> &) const;
    void print_solution(const std::vector<Node *> &) const;
};

static_assert(std::is_move_constructible<DLXMatrix>::value);
static_assert(std::is_move_assignable<DLXMatrix>::value);

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v);
std::ostream &operator<<(std::ostream &out, const DLXMatrix &M);

}  // namespace DLX_backtrack

namespace std {

inline ostream &operator<<(ostream &out, const DLX_backtrack::DLXMatrix &M) {
    return DLX_backtrack::operator<<(out, M);
}

}  // namespace std

#endif  // DLX_MATRIX_HPP_
