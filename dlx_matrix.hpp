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
#ifndef DLX_MATRIX_HPP_
#define DLX_MATRIX_HPP_

#include <iostream>
#include <limits>      // numeric_limits
#include <vector>

namespace DLX_backtrack {

/////////////////
class DLXMatrix {
 public:
  using ind_t = int;

 private:
  struct Node {
    int top;
    int up, down;

    int &size() { return top; }
    int size() const { return top; }
  };

  struct Header {
    int left, right;
  };

  int nb_primary_;
  std::vector<Header> heads_;
  std::vector<Node> rows_;

  std::vector<int> work_;
  bool search_down_;

 public:
  using Vect1D = std::vector<ind_t>;
  using Vect2D = std::vector<Vect1D>;

  DLXMatrix() : DLXMatrix(0) {}
  explicit DLXMatrix(ind_t nb_col) : DLXMatrix(nb_col, nb_col) {}
  DLXMatrix(ind_t nb_col, ind_t nb_primary);
  DLXMatrix(ind_t nb_col, const Vect2D &rows)
      : DLXMatrix(nb_col, nb_col, rows) {}
  DLXMatrix(ind_t nb_col, ind_t nb_primary, const Vect2D &rows);
  DLXMatrix(const DLXMatrix &) = default;
  DLXMatrix &operator=(const DLXMatrix &other) = default;
  DLXMatrix(DLXMatrix &&) = default;
  DLXMatrix &operator=(DLXMatrix &&other) = default;
  ~DLXMatrix() = default;

  int nb_cols() const { return heads_.size() - 1; }
  int nb_rows() const { return -rows_.back().top; }
  int nb_primary() const { return nb_primary_; }

  void check_sizes() const;

  ind_t add_row(const Vect1D &r) { return add_row_sparse(r); }
  ind_t add_row_sparse(const Vect1D &r);
  ind_t add_row_dense(const std::vector<bool> &r);
  Vect1D row_sparse(ind_t i) const;
  std::vector<bool> row_dense(ind_t i) const;

  Vect1D row_to_sparse(const std::vector<bool> &row) const;
  std::vector<bool> row_to_dense(Vect1D row) const;

  Vect2D search_rec(size_t max_sol = std::numeric_limits<size_t>::max());
  bool search_iter();
  bool search_iter(Vect1D &);
  Vect1D get_solution(bool sorted = true);
  bool search_random(Vect1D &);

  bool is_solution(const Vect1D &);

  void reset();

  DLXMatrix permuted_columns(const Vect1D &perm);
  DLXMatrix permuted_inv_columns(const Vect1D &perm);
  DLXMatrix permuted_rows(const Vect1D &perm);

  std::string to_string() const;

  int nb_choices, nb_dances;  // Computation statistics

  void debug();

 protected:
  Header *master() { return &heads_[0]; }
  const Header *master() const { return &heads_[0]; }

  ind_t get_col_id(int h) const {
    return (h > 0) ? h - 1 : std::numeric_limits<ind_t>::max();
  }
  bool is_primary(int h) const { return h != 0 && h <= nb_primary_; }

  int next_in_row(int nd) {
    ++nd;
    return rows_[nd].top <= 0 ? rows_[nd].up : nd;
  }
  int prev_in_row(int nd) {
    --nd;
    return rows_[nd].top <= 0 ? rows_[nd].down : nd;
  }
  int row_id(int nd) {
    while (rows_[nd].top > 0) nd--;
    return -rows_[nd].top;
  }
  int choose_min();
  void hide(int row);
  void unhide(int row);
  void cover(int col);
  void uncover(int col);
  void choose(int nd);
  void unchoose(int nd);
  void search_rec_internal(size_t, Vect2D &);

  void print_solution(const std::vector<int> &) const;
};

struct size_mismatch_error : public std::runtime_error {
  size_mismatch_error(const std::string &s, DLXMatrix::ind_t expected,
                      DLXMatrix::ind_t sz)
      : std::runtime_error("Wrong " + s + " size: " + std::to_string(sz) +
                           " (expecting " + std::to_string(expected) + ")") {}
};
void check_size(const std::string &s, DLXMatrix::ind_t expected,
                DLXMatrix::ind_t sz);

struct empty_error : public std::runtime_error {
  explicit empty_error(const std::string &s)
      : std::runtime_error("Empty " + s + " are not allowed") {}
};

std::vector<DLXMatrix::ind_t> inverse_perm(
    const std::vector<DLXMatrix::ind_t> &perm);

static_assert(std::is_move_constructible<DLXMatrix>::value,
              "DLXMatrix should be move constructible");
static_assert(std::is_move_assignable<DLXMatrix>::value,
              "DLXMatrix should bt move assignable");

inline std::ostream &operator<<(std::ostream &out,
                                const DLX_backtrack::DLXMatrix &M) {
  return out << M.to_string();
}

}  // namespace DLX_backtrack

#endif  // DLX_MATRIX_HPP_
