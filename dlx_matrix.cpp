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
#include "dlx_matrix.hpp"

#include "doctest_ext.hpp"

#include <algorithm>  // sort, transform, shuffle
#include <iostream>   // cout, cin, ...
#include <limits>     // numeric_limits
#include <numeric>    // iota
#include <random>     // default_random_engine
#include <sstream>    // ostringstream
#include <stdexcept>  // out_of_range
#include <vector>     // vector

namespace DLX_backtrack {

using Vect1D = DLXMatrix::Vect1D;
using Vect2D = DLXMatrix::Vect2D;
using ind_t = DLXMatrix::ind_t;

/////////////////////////////////////////
TEST_SUITE_END();  // [dlx_matrix]Errors;
/////////////////////////////////////////

Vect1D inverse_perm(const Vect1D &perm) {
  Vect1D inv(perm.size());
  for (size_t i = 0; i < perm.size(); i++) inv[perm[i]] = i;
  return inv;
}

TEST_CASE("[dlx_matrix]Function inverse_perm") {
  CHECK(inverse_perm({}) == Vect1D({}));
  CHECK(inverse_perm({0}) == Vect1D{0});
  CHECK(inverse_perm({0, 1}) == Vect1D{0, 1});
  CHECK(inverse_perm({1, 0}) == Vect1D{1, 0});
  CHECK(inverse_perm({1, 0, 3, 2}) == Vect1D{1, 0, 3, 2});
  CHECK(inverse_perm({1, 3, 0, 2}) == Vect1D{2, 0, 3, 1});
  CHECK(inverse_perm({6, 1, 5, 3, 7, 0, 4, 2}) ==
        Vect1D{5, 1, 7, 3, 6, 2, 0, 4});
}

////////////////////////////////////////////////
TEST_SUITE_BEGIN("[dlx_matrix]class DLXMatrix");
////////////////////////////////////////////////

class DLXMatrixFixture {
 public:
  // clang-format off
    DLXMatrixFixture() :
// MA2AB
//     0 1 2 3 4 5 6 7 8 9
//  0 [1 0 0 0 1 0 0 0 0 0]
//  1 [1 0 0 0 0 1 0 0 0 0]
//  2 [1 0 0 0 0 0 1 0 0 0]
//  3 [0 1 0 0 1 0 0 0 0 0]
//  4 [0 1 0 0 0 1 0 0 0 0]
//  5 [0 1 0 0 0 0 1 0 0 0]
//  6 [0 0 1 0 1 0 0 0 0 1]
//  7 [0 0 1 0 0 1 0 0 0 0]
//  8 [0 0 1 0 0 0 1 0 0 0]
//  9 [0 0 0 1 0 0 0 1 0 1]
// 10 [0 1 0 0 0 1 0 0 1 0]
        VA2AB{{0, 4}, {0, 5}, {0, 6}, {1, 4}, {1, 5}, {1, 6},
              {2, 4, 9}, {2, 5}, {2, 6}, {3, 7, 9}, {1, 5, 8}},

        empty0(0), empty1(1), empty5(5),
        M1_1(1, {{0}}),
        M5_2(5, {{0, 1}, {2, 3, 4}}),
        M5_3(5, {{0, 1}, {2, 3, 4}, {1, 2, 4}}),
        M5_3_Sec2(5, 3, {{0, 1}, {2}, {2, 3, 4}, {1, 2, 4}}),
        M6_10(6, { {0, 2}, {0, 1}, {1, 4}, {3}, {3, 4}, {5},
                   {1}, {0, 1, 2}, {2, 3, 4}, {1, 4, 5} }),
        MA2AB(10, 9, VA2AB), MA2AB_8(10, 8, VA2AB),

        TestSample({empty0, empty1, empty5, M1_1, M5_2, M5_3, M5_3_Sec2, M6_10,
                    MA2AB, MA2AB_8}) {}
  // clang-format on
 protected:
  DLXMatrix::Vect2D VA2AB;
  DLXMatrix empty0, empty1, empty5, M1_1, M5_2, M5_3, M5_3_Sec2, M6_10, MA2AB,
      MA2AB_8;
  const std::string M5_3_Sec2_str =
      "[1, 1, 0 | 0, 0]\n"
      "[0, 0, 1 | 0, 0]\n"
      "[0, 0, 1 | 1, 1]\n"
      "[0, 1, 1 | 0, 1]\n";
  std::vector<DLXMatrix> TestSample;
};

std::string DLXMatrix::to_string() const {
  std::string res;
  for (const auto &row : rows_) {
    auto r = row_dense(row);
    res += "[" + std::to_string(static_cast<int>(r[0]));
    for (size_t i = 1; i < r.size(); ++i) {
      res += i == nb_primary_ ? " | " : ", ";
      res += std::to_string(static_cast<int>(r[i]));
    }
    res += "]\n";
  }
  return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method to_string") {
  CHECK(M5_2.to_string() == "[1, 1, 0, 0, 0]\n[0, 0, 1, 1, 1]\n");
  CHECK(M5_3_Sec2.to_string() == M5_3_Sec2_str);
}

TEST_CASE_FIXTURE(DLXMatrixFixture, "DLXMatrix::operator<<") {
  std::ostringstream os;
  os << M5_3_Sec2;
  CHECK(os.str() == M5_3_Sec2_str);
}

DLXMatrix::DLXMatrix(ind_t nb_col, ind_t nb_primary)
    : nb_primary_(std::min(nb_col, nb_primary)),
      depth_(0),
      heads_(nb_col + 1),
      search_down_(true),
      nb_choices(0),
      nb_dances(0) {
  for (ind_t i = 0; i <= nb_col; i++) {
    heads_[i].size = 0;
    heads_[i].node.up = heads_[i].node.down = &heads_[i].node;
    // heads_[i].node.head = &heads_[i];  // unused
    // heads_[i].node.row_id = -1  // unused;
    // heads_[i].node.left = heads_[i].node.right = nullptr;  // unused
  }
  heads_[nb_col].right = &heads_[0];
  for (ind_t i = 0; i < nb_col; i++) heads_[i].right = &heads_[i + 1];
  heads_[0].left = &heads_[nb_col];
  for (ind_t i = 1; i <= nb_col; i++) heads_[i].left = &heads_[i - 1];
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Constructor DLXMatrix(ind_t)") {
  CHECK(empty0.nb_cols() == 0);
  CHECK(empty0.nb_rows() == 0);
  CHECK(empty5.nb_cols() == 5);
  CHECK(empty5.nb_rows() == 0);
}
TEST_CASE("Constructor DLXMatrix(ind_t, ind_t)") {
  CHECK_NOTHROW(DLXMatrix(0, 0));
  DLXMatrix M00(0, 0);
  CHECK(M00.nb_cols() == 0);
  CHECK(M00.nb_primary() == 0);
  CHECK_NOTHROW(DLXMatrix(5, 2));
  DLXMatrix M52(5, 2);
  CHECK(M52.nb_cols() == 5);
  CHECK(M52.nb_primary() == 2);
  CHECK_NOTHROW(DLXMatrix(5, 6));
  DLXMatrix M56(5, 6);
  CHECK(M56.nb_cols() == 5);
  CHECK(M56.nb_primary() == 5);
}

DLXMatrix::DLXMatrix(ind_t nb_col, ind_t nb_primary, const Vect2D &rows)
    : DLXMatrix(nb_col, nb_primary) {
  for (const auto &r : rows) add_row_sparse(r);
}
TEST_CASE_FIXTURE(DLXMatrixFixture,
                  "Constructor DLXMatrix(ind_t, const Vect2D &))") {
  CHECK(M5_2.nb_cols() == 5);
  CHECK(M5_2.nb_rows() == 2);
  CHECK(M6_10.nb_cols() == 6);
  CHECK(M6_10.nb_rows() == 10);
}
TEST_CASE("Constructor DLXMatrix(ind_t, ind_t, const Vect2D &))") {
  CHECK_NOTHROW(DLXMatrix(0, 0, {}));
  CHECK_NOTHROW(DLXMatrix(1, 1, {{0}}));
  CHECK_NOTHROW(DLXMatrix(3, 2, {{0}, {1, 2}}));
}

DLXMatrix::DLXMatrix(const DLXMatrix &other)
    : DLXMatrix(other.nb_cols(), other.nb_primary_) {
  for (const auto &row : other.rows_) add_row_sparse(other.row_sparse(row));
  for (const Node *nother : other.work_) {
    ind_t id = other.get_row_id(nother);
    Node *node = &rows_[id][std::distance(other.rows_[id].data(), nother)];
    cover(node->head);
    choose(node);
  }
  search_down_ = other.search_down_;
  nb_choices = other.nb_choices;
  nb_dances = other.nb_dances;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "DLXMatrix copy constructor") {
  for (const DLXMatrix &M : TestSample) {
    CAPTURE(M);
    DLXMatrix N(M);
    CHECK(N.nb_cols() == M.nb_cols());
    REQUIRE(N.nb_rows() == M.nb_rows());
    for (ind_t i = 0; i < M.nb_rows(); i++)
      CHECK(N.ith_row_sparse(i) == M.ith_row_sparse(i));
    // Check that modifying the copy doesn't change the original
    if (N.nb_cols() != 0) {
      N.add_row_sparse({0});
      REQUIRE(N.nb_rows() == M.nb_rows() + 1);
    }
  }
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "DLXMatrix move constructor/assignement") {
  DLXMatrix M = std::move(M5_3);
  CHECK(M.nb_cols() == 5);

  M = std::move(M6_10);
  CHECK(M.nb_cols() == 6);

  std::swap(empty0, empty5);
  CHECK(empty0.nb_cols() == 5);
  CHECK(empty5.nb_cols() == 0);
}

DLXMatrix &DLXMatrix::operator=(const DLXMatrix &other) {
  DLXMatrix res(other);
  nb_primary_ = res.nb_primary_;
  heads_ = std::move(res.heads_);
  rows_ = std::move(res.rows_);
  work_ = std::move(res.work_);
  search_down_ = res.search_down_;
  nb_choices = res.nb_choices;
  nb_dances = res.nb_dances;
  return *this;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "DLXMatrix::operator=") {
  for (const DLXMatrix &M : TestSample) {
    CAPTURE(M);
    DLXMatrix N(0);
    N = M;
    CHECK(N.nb_cols() == M.nb_cols());
    REQUIRE(N.nb_rows() == M.nb_rows());
    for (ind_t i = 0; i < M.nb_rows(); i++)
      CHECK(N.ith_row_sparse(i) == M.ith_row_sparse(i));
  }
}

Vect1D DLXMatrix::row_sparse(const std::vector<Node> &row) const {
  return details::vector_transform(
      row, [this](const Node &n) -> ind_t { return get_col_id(n.head); });
}
Vect1D DLXMatrix::ith_row_sparse(ind_t i) const {
  return row_sparse(rows_[i]);
}
TEST_CASE("Method ith_row_sparse") {
  DLXMatrix M(5, {{0, 1}, {2, 3, 4}, {1, 2, 4}});
  CHECK(M.ith_row_sparse(0) == Vect1D({0, 1}));
  CHECK(M.ith_row_sparse(1) == Vect1D({2, 3, 4}));
  CHECK(M.ith_row_sparse(2) == Vect1D({1, 2, 4}));
}

std::vector<bool> DLXMatrix::row_dense(const std::vector<Node> &row) const {
  return row_to_dense(DLXMatrix::row_sparse(row));
}
std::vector<bool> DLXMatrix::ith_row_dense(ind_t i) const {
  return row_dense(rows_[i]);
}
TEST_CASE("Method ith_row_dense") {
  DLXMatrix M(5, {{0, 1}, {2, 3, 4}, {1, 2, 4}});
  CHECK(M.ith_row_dense(0) == std::vector<bool>({1, 1, 0, 0, 0}));
  CHECK(M.ith_row_dense(1) == std::vector<bool>({0, 0, 1, 1, 1}));
  CHECK(M.ith_row_dense(2) == std::vector<bool>({0, 1, 1, 0, 1}));
}

void DLXMatrix::check_sizes() const {
  for (Header *h = master()->right; h != master(); h = h->right) {
    ind_t irows = 0;
    for (Node *p = h->node.down; p != &h->node; p = p->down) irows++;
    if (h->size != irows)
      throw std::runtime_error("check_size : size missmatch !");
  }
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method check_sizes") {
  for (const DLXMatrix &M : TestSample) {
    CAPTURE(M);
    CHECK_NOTHROW(M.check_sizes());
  }
}

ind_t DLXMatrix::add_row_sparse(const Vect1D &r) {
  // Assume that the row is not empty and correct
  // if (r.empty()) throw empty_error("rows");
  // Check for bound before modifying anything
  // for (ind_t i : r) heads_.at(i + 1);

  ind_t row_id = rows_.size();
  rows_.emplace_back(r.size());
  std::vector<Node> &row = rows_.back();

  for (size_t i = 0; i < r.size(); i++) {
    auto &h = heads_[r[i] + 1];
    h.size++;
    row[i].row_id = row_id;
    row[i].head = &h;
    row[i].down = &h.node;
    row[i].up = h.node.up;
    row[i].up->down = h.node.up = &row[i];
  }
  row.back().right = &row[0];
  for (size_t i = 0; i < r.size() - 1; i++) row[i].right = &row[i + 1];
  row[0].left = &row.back();
  for (size_t i = 1; i < r.size(); i++) row[i].left = &row[i - 1];
  return row_id;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method add_row_sparse") {
  SUBCASE("Correct row is added") {
    for (auto &M : TestSample) {
      CAPTURE(M);
      DLXMatrix MSave(M);
      if (M.nb_cols() >= 4) {
        ind_t rid = M.add_row_sparse({2, 3});
        CHECK(rid == MSave.nb_rows());
        CHECK(M.nb_cols() == MSave.nb_cols());
        REQUIRE(M.nb_rows() == MSave.nb_rows() + 1);
        for (ind_t i = 0; i < MSave.nb_rows(); i++)
          CHECK(M.ith_row_sparse(i) == MSave.ith_row_sparse(i));
        CHECK(M.ith_row_sparse(MSave.nb_rows()) == Vect1D({2, 3}));
        CHECK_NOTHROW(M.check_sizes());
      }
    }
  }
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method add_row") {
  CHECK(M5_3.add_row({2, 3}) == 3);
  CHECK(M5_3.ith_row_sparse(3) == Vect1D({2, 3}));
}

Vect1D DLXMatrix::row_to_sparse(const std::vector<bool> &row) const {
  // No check is performed
  // check_size("row", nb_cols(), row.size());
  Vect1D res;
  for (size_t i = 0; i < row.size(); i++)
    if (row[i]) res.push_back(i);
  return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method row_to_sparse") {
  CHECK(M5_3.row_to_sparse({0, 1, 1, 0, 0}) == Vect1D({1, 2}));
  CHECK(M5_3.row_to_sparse({0, 1, 0, 1, 1}) == Vect1D({1, 3, 4}));
}

std::vector<bool> DLXMatrix::row_to_dense(Vect1D row) const {
  // No check for bound
  // for (ind_t i : row) heads_.at(i + 1);
  std::sort(row.begin(), row.end());
  std::vector<bool> res(nb_cols(), false);
  auto it = row.cbegin(), end = row.cend();
  for (ind_t i = 0; it < end; i++) {
    if (i == *it) {
      res[i] = true;
      it++;
    }
  }
  return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method row_to_dense") {
  CHECK(M5_3.row_to_dense({1, 2}) == std::vector<bool>({0, 1, 1, 0, 0}));
  CHECK(M5_3.row_to_dense({1, 3, 4}) == std::vector<bool>({0, 1, 0, 1, 1}));
}

ind_t DLXMatrix::add_row_dense(const std::vector<bool> &r) {
  return add_row_sparse(row_to_sparse(r));
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method add_row_dense") {
  CHECK(M5_3.add_row_dense({0, 0, 1, 1, 0}) == 3);
  CHECK(M5_3.ith_row_sparse(3) == Vect1D({2, 3}));
}

bool DLXMatrix::is_solution(const Vect1D &sol) const {
  Vect1D cols(nb_cols());
  for (ind_t r : sol) {
    std::transform(cols.begin(), cols.end(), ith_row_dense(r).begin(),
                   cols.begin(), std::plus<>());
  }
  for (ind_t i = 0; i < nb_primary_; i++)
    if (cols[i] != 1) return false;
  for (ind_t i = nb_primary_; i < nb_cols(); i++)
    if (cols[i] > 1) return false;
  return true;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method is_solution") {
  SUBCASE("Empty 0x0 Matrix") {
    CHECK(empty0.is_solution({}));
  }
  SUBCASE("Empty 0x5 Matrix") {
    CHECK_FALSE(empty5.is_solution({}));
  }
  SUBCASE("M5_2 Matrix") {
    CHECK_FALSE(M5_2.is_solution({}));
    CHECK_FALSE(M5_2.is_solution({0}));
    CHECK_FALSE(M5_2.is_solution({1}));
    CHECK(M5_2.is_solution({0, 1}));
    CHECK(M5_2.is_solution({1, 0}));
  }
  SUBCASE("M5_3 Matrix") {
    CHECK(M5_3.is_solution({0, 1}));
    for (const auto &s : {Vect1D{}, {0}, {1}, {2}, {0, 2}, {1, 2}, {0, 1, 2}}) {
      CAPTURE(s);
      CHECK_FALSE(M5_3.is_solution(s));
    }
  }
  SUBCASE("M6_10 Matrix") {
    CHECK(M6_10.is_solution({0, 4, 5, 6}));
    CHECK(M6_10.is_solution({6, 0, 5, 4}));
    CHECK_FALSE(M6_10.is_solution({0, 2, 4, 5, 6}));
    CHECK_FALSE(M6_10.is_solution({0, 5, 6}));
  }
  SUBCASE("M5_3_Sec2 Matrix (2 secondary columns") {
    CHECK(M5_3_Sec2.is_solution({0, 1}));
    CHECK(M5_3_Sec2.is_solution({0, 2}));
    CHECK_FALSE(M5_3_Sec2.is_solution({0}));
    CHECK_FALSE(M5_3_Sec2.is_solution({0, 3}));
  }
  SUBCASE("MA2AB (1 secondary columns") {
    CHECK(MA2AB.is_solution({0, 8, 9, 10}));
    CHECK(MA2AB.is_solution({8, 9, 0, 10}));
    CHECK_FALSE(MA2AB.is_solution({0, 1}));
    CHECK_FALSE(MA2AB.is_solution({0, 2}));
    CHECK_FALSE(MA2AB.is_solution({0}));
    CHECK_FALSE(MA2AB.is_solution({0, 8, 9}));
  }
}

bool DLXMatrix::is_active(const Header *h) const {
  for (Header *c = master()->right; c != master(); c = c->right)
    if (c == h) return true;
  return false;
}
static void check_col_active(const DLXMatrix &M, const std::vector<int> &Sol) {
  REQUIRE(M.nb_cols() == Sol.size());
  for (ind_t i = 0; i < M.nb_cols(); i++) {
    CAPTURE(i);
    CHECK(M.is_col_active(i) == Sol[i]);
  }
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method is_col_active") {
  // M6_10(6, { {0, 2}, {0, 1}, {1, 4}, {3}, {3, 4}, {5},
  //            {1}, {0, 1, 2}, {2, 3, 4}, {1, 4, 5} })
  check_col_active(M6_10, {1, 1, 1, 1, 1, 1});
  M6_10.choose(2);
  check_col_active(M6_10, {1, 0, 1, 1, 0, 1});
  M6_10.choose(5);
  check_col_active(M6_10, {1, 0, 1, 1, 0, 0});
}

bool DLXMatrix::is_active(const Node *nd) const {
  Header *h = nd->head;
  if (!is_active(h)) return false;
  for (Node *row = h->node.down; row != &h->node; row = row->down)
    if (nd == row) return true;
  return false;
}
static void check_row_active(const DLXMatrix &M, const std::vector<int> &Sol) {
  REQUIRE(M.nb_rows() == Sol.size());
  for (ind_t i = 0; i < M.nb_rows(); i++) {
    CAPTURE(i);
    CHECK(M.is_row_active(i) == Sol[i]);
  }
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method is_row_active") {
  // M6_10(6, { {0, 2}, {0, 1}, {1, 4}, {3}, {3, 4}, {5},
  //            {1}, {0, 1, 2}, {2, 3, 4}, {1, 4, 5} })
  check_row_active(M6_10, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
  M6_10.choose(2);
  check_row_active(M6_10, {1, 0, 0, 1, 0, 1, 0, 0, 0, 0});
  M6_10.choose(5);
  check_row_active(M6_10, {1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
}

inline void DLXMatrix::hide(Node *row) {
  for (Node *nr = row->right; nr != row; nr = nr->right) {
    nr->up->down = nr->down;
    nr->down->up = nr->up;
    nr->head->size--;
    nb_dances++;
  }
}
inline void DLXMatrix::cover(Header *col) {
  col->left->right = col->right;
  col->right->left = col->left;
  for (Node *row = col->node.down; row != &col->node; row = row->down) {
    hide(row);
  }
}
inline void DLXMatrix::choose(Node *nd) {
  nb_choices++;
  work_.push_back(nd);
  for (Node *nr = nd->right; nr != nd; nr = nr->right) cover(nr->head);
}

inline void DLXMatrix::unhide(Node *row) {
  for (Node *nr = row->left; nr != row; nr = nr->left) {
    nr->head->size++;
    nr->up->down = nr;
    nr->down->up = nr;
  }
}
inline void DLXMatrix::uncover(Header *col) {
  col->left->right = col;
  col->right->left = col;

  for (Node *row = col->node.up; row != &col->node; row = row->up) {
    unhide(row);
  }
}
inline void DLXMatrix::unchoose(Node *nd) {
  for (Node *nr = nd->left; nr != nd; nr = nr->left) uncover(nr->head);
  work_.pop_back();
}

DLXMatrix::Header *DLXMatrix::choose_min() {
  Header *choice = master()->right;
  ind_t min_size = choice->size;
  for (Header *h = choice->right; is_primary(h); h = h->right) {
    if (h->size < min_size) {
      choice = h;
      min_size = h->size;
    }
  }
  return choice;
}

// Knuth dancing links search algorithm
// Recusive version
///////////////////////////////////////
Vect2D DLXMatrix::search_rec(size_t max_sol) {
  Vect2D res{};
  nb_choices = nb_dances = 0;
  search_rec_internal(max_sol, res);
  return res;
}
void DLXMatrix::search_rec_internal(size_t max_sol, Vect2D &res) {
  if (!is_primary(master()->right)) {
    res.push_back(get_solution());
    return;
  }

  Header *choice = choose_min();
  if (choice->size == 0) return;

  cover(choice);
  for (Node *row = choice->node.down; row != &choice->node; row = row->down) {
    choose(row);
    search_rec_internal(max_sol, res);
    unchoose(row);
    if (res.size() >= max_sol) break;
  }
  uncover(choice);
}

DLXMatrix::Vect2D normalize_solutions(DLXMatrix::Vect2D sols) {
  for (auto &sol : sols) std::sort(sol.begin(), sol.end());
  std::sort(sols.begin(), sols.end());
  return sols;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method search_rec") {
  SUBCASE("Basic matrices") {
    CHECK(
        normalize_solutions(M6_10.search_rec()) ==
        Vect2D({{0, 2, 3, 5}, {0, 3, 9}, {0, 4, 5, 6}, {1, 5, 8}, {4, 5, 7}}));
    CHECK(normalize_solutions(M5_2.search_rec()) == Vect2D({{0, 1}}));
    CHECK(normalize_solutions(M5_3.search_rec()) == Vect2D({{0, 1}}));
  }
  SUBCASE("Matrices with secondary columns") {
    CHECK(normalize_solutions(M5_3_Sec2.search_rec()) ==
          Vect2D({{0, 1}, {0, 2}}));
    CHECK(normalize_solutions(MA2AB.search_rec()) == Vect2D({{0, 8, 9, 10}}));
    CHECK(normalize_solutions(MA2AB_8.search_rec()) == Vect2D({{0, 4, 8, 9},
                                                               {0, 5, 7, 9},
                                                               {0, 8, 9, 10},
                                                               {1, 3, 8, 9},
                                                               {2, 3, 7, 9}}));
  }

  SUBCASE("Check that all found solutions are actual solutions") {
    for (DLXMatrix &M : TestSample) {
      for (const Vect1D &s : M.search_rec()) {
        CAPTURE(M);
        CAPTURE(s);
        CHECK(M.is_solution(s));
      }
    }
  }
}

// Knuth dancing links search algorithm
// Iterative version
///////////////////////////////////////
bool DLXMatrix::search_iter() {
  while (search_down_ || work_.size() > depth_) {
    if (search_down_) {  // going down the recursion
      if (!is_primary(master()->right)) {
        search_down_ = false;
        return true;
      }
      Header *choice = choose_min();
      if (choice->size == 0) {
        search_down_ = false;
      } else {
        cover(choice);
        choose(choice->node.down);
      }
    } else {  // going up the recursion
      Node *row = work_.back();
      Header *choice = row->head;
      unchoose(row);
      row = row->down;
      if (row != &choice->node) {
        choose(row);
        search_down_ = true;
      } else {
        uncover(choice);
      }
    }
  }
  return false;
}
bool DLXMatrix::search_iter(Vect1D &v) {
  bool res;
  if ((res = search_iter())) v = get_solution();
  return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method search_iter") {
  SUBCASE("agrees with search_rec") {
    for (auto M : TestSample) {
      CAPTURE(M);
      DLXMatrix MSave(M);
      DLXMatrix::Vect2D sols;
      while (M.search_iter()) sols.push_back(M.get_solution());
      CHECK(sols == MSave.search_rec());
    }
  }
  SUBCASE("still work after copy") {
    REQUIRE(M6_10.search_iter());  // consumes one solution
    REQUIRE(M6_10.search_iter());  // consumes one solution
    DLXMatrix N(M6_10);
    Vect2D solM, solN;
    while (M6_10.search_iter()) solM.push_back(M6_10.get_solution());
    while (N.search_iter()) solN.push_back(N.get_solution());
    CHECK(solN == solM);
  }
  SUBCASE("still work after assignment") {
    REQUIRE(M6_10.search_iter());  // consumes one solution
    REQUIRE(M6_10.search_iter());  // consumes one solution
    DLXMatrix N(0);
    N = M6_10;
    Vect2D solM, solN;
    while (M6_10.search_iter()) solM.push_back(M6_10.get_solution());
    while (N.search_iter()) solN.push_back(N.get_solution());
    CHECK(solN == solM);
  }
}

Vect1D DLXMatrix::get_solution() {
  return details::vector_transform(
      work_, [this](Node *n) -> ind_t { return get_row_id(n); });
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method get_solution") {
  CHECK(M6_10.get_solution() == Vect1D({}));
  REQUIRE(M6_10.search_iter());
  CHECK(M6_10.get_solution() == Vect1D({5, 0, 2, 3}));
  REQUIRE(M6_10.search_iter());
  CHECK(M6_10.get_solution() == Vect1D({5, 0, 6, 4}));
}

void DLXMatrix::reset(size_t depth) {
  nb_choices = nb_dances = 0;
  while (work_.size() > depth) {
    Node *row = work_.back();
    unchoose(row);
    uncover(row->head);
  }
  search_down_ = true;
  depth_ = work_.size();
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method reset") {
  DLXMatrix N(M6_10);
  REQUIRE(N.search_iter());  // consumes one solution
  REQUIRE(N.search_iter());  // consumes one solution
  N.reset();
  Vect2D solM, solN;
  while (M6_10.search_iter()) solM.push_back(M6_10.get_solution());
  while (N.search_iter()) solN.push_back(N.get_solution());
  CHECK(solN == solM);
}

ind_t DLXMatrix::choose(ind_t i) {
  Node *nd = rows_[i].data();
  cover(nd->head);
  choose(nd);
  return ++depth_;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method choose") {
  CHECK(M6_10.choose(2) == 1);
  Vect2D solM;
  while (M6_10.search_iter()) solM.push_back(M6_10.get_solution());
  CHECK(normalize_solutions(solM) == Vect2D({{0, 2, 3, 5}}));
  M6_10.reset();
  solM.clear();
  CHECK(M6_10.choose(5) == 1);
  CHECK(M6_10.choose(4) == 2);
  while (M6_10.search_iter()) solM.push_back(M6_10.get_solution());
  CHECK(normalize_solutions(solM) == Vect2D({{0, 4, 5, 6}, {4, 5, 7}}));
  M6_10.reset(1);
  solM.clear();
  while (M6_10.search_iter()) solM.push_back(M6_10.get_solution());
  CHECK(normalize_solutions(solM) ==
        Vect2D({{0, 2, 3, 5}, {0, 4, 5, 6}, {1, 5, 8}, {4, 5, 7}}));
}

DLXMatrix DLXMatrix::permuted_inv_columns(const Vect1D &perm) const {
  // No check is performed
  // check_size("permutation", perm.size(), nb_cols());
  DLXMatrix res(nb_cols());
  for (const auto &row : rows_) {
    res.add_row_sparse(details::vector_transform(
        row, [this, &perm](const Node &n) -> ind_t {
          return perm[get_col_id(n.head)];
        }));
  }
  return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method permuted_inv_columns") {
  SUBCASE("empty0") {
    Vect1D perm{};
    CHECK(empty0.permuted_inv_columns(perm).nb_cols() == 0);
    CHECK(empty0.permuted_inv_columns(perm).nb_rows() == 0);
  }
  SUBCASE("empty5") {
    Vect1D perm{3, 4, 0, 2, 1};
    CHECK(empty5.permuted_inv_columns(perm).nb_cols() == 5);
    CHECK(empty5.permuted_inv_columns(perm).nb_rows() == 0);
  }
  SUBCASE("M6_10") {
    Vect1D perm{4, 3, 2, 0, 5, 1};
    DLXMatrix M = M6_10.permuted_inv_columns(perm);
    CHECK(M.nb_cols() == M6_10.nb_cols());
    CHECK(M.nb_rows() == M6_10.nb_rows());
    CHECK(normalize_solutions(M.search_rec()) ==
          normalize_solutions(M6_10.search_rec()));
    for (ind_t r = 0; r < M6_10.nb_rows(); r++) {
      for (ind_t c = 0; c < M6_10.nb_cols(); c++) {
        CHECK(M.ith_row_dense(r)[perm[c]] == M6_10.ith_row_dense(r)[c]);
      }
    }
  }
}
DLXMatrix DLXMatrix::permuted_columns(const Vect1D &perm) const {
  return permuted_inv_columns(inverse_perm(perm));
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method permuted_columns") {
  SUBCASE("empty0") {
    Vect1D perm{};
    CHECK(empty0.permuted_columns(perm).nb_cols() == 0);
    CHECK(empty0.permuted_columns(perm).nb_rows() == 0);
  }
  SUBCASE("empty5") {
    Vect1D perm{3, 4, 0, 2, 1};
    CHECK(empty5.permuted_columns(perm).nb_cols() == 5);
    CHECK(empty5.permuted_columns(perm).nb_rows() == 0);
  }
  SUBCASE("M6_10") {
    Vect1D perm{4, 3, 2, 0, 5, 1};
    DLXMatrix M = M6_10.permuted_columns(perm);
    CHECK(M.nb_cols() == M6_10.nb_cols());
    CHECK(M.nb_rows() == M6_10.nb_rows());
    CHECK(normalize_solutions(M.search_rec()) ==
          normalize_solutions(M6_10.search_rec()));
    for (ind_t r = 0; r < M6_10.nb_rows(); r++) {
      for (ind_t c = 0; c < M6_10.nb_cols(); c++) {
        CHECK(M.ith_row_dense(r)[c] == M6_10.ith_row_dense(r)[perm[c]]);
      }
    }
  }
}

DLXMatrix DLXMatrix::permuted_rows(const Vect1D &perm) const {
  // if (perm.size() != nb_rows())
  //    throw size_mismatch_error("permutation", perm.size(), nb_rows());
  DLXMatrix res(nb_cols());
  for (ind_t i : perm) res.add_row_sparse(row_sparse(rows_[i]));
  return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method permuted_rows") {
  SUBCASE("empty") {
    Vect1D perm{};
    CHECK(empty0.permuted_rows(perm).nb_cols() == 0);
    CHECK(empty0.permuted_rows(perm).nb_rows() == 0);
    CHECK(empty5.permuted_rows(perm).nb_cols() == 5);
    CHECK(empty5.permuted_rows(perm).nb_rows() == 0);
  }
  SUBCASE("M6_10") {
    Vect1D perm{4, 7, 3, 8, 2, 0, 5, 1, 9, 6};
    DLXMatrix M = M6_10.permuted_rows(perm);
    CHECK(M.nb_cols() == M6_10.nb_cols());
    CHECK(M.nb_rows() == M6_10.nb_rows());
    for (ind_t i = 0; i < 10; i++) {
      CAPTURE(i);
      CHECK(M.ith_row_sparse(i) == M6_10.ith_row_sparse(perm[i]));
    }
  }
}

bool DLXMatrix::search_random(Vect1D &sol) {
  static std::mt19937 rng{std::random_device {}()};

  Vect1D row_perm(nb_rows());
  std::iota(row_perm.begin(), row_perm.end(), 0);
  std::shuffle(row_perm.begin(), row_perm.end(), rng);

  Vect1D col_perm(nb_cols());
  std::iota(col_perm.begin(), col_perm.end(), 0);
  std::shuffle(col_perm.begin(), col_perm.begin() + nb_primary_, rng);

  DLXMatrix M = permuted_inv_columns(col_perm).permuted_rows(row_perm);
  if (!M.search_iter()) return false;
  Vect1D v = M.get_solution();
  sol.resize(v.size());
  for (size_t i = 0; i < v.size(); i++) sol[i] = row_perm[v[i]];
  return true;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "Method search_random") {
  for (DLXMatrix &M : TestSample) {
    CAPTURE(M);
    Vect1D sol;
    bool has_sol = M.search_iter();
    REQUIRE(M.search_random(sol) == has_sol);
    if (has_sol) {
      for (ind_t i = 0; i < 20; i++) {
        REQUIRE(M.search_random(sol));
        CAPTURE(sol);
        CHECK(M.is_solution(sol));
      }
    }
  }
}

////////////////////////////////////////////////////
TEST_SUITE_END();  // "[dlx_matrix]class DLXMatrix";
////////////////////////////////////////////////////

/////////////////////////////////////////////////////
TEST_SUITE_BEGIN("[dlx_matrix]class DLXMatrixNamed");
/////////////////////////////////////////////////////

TEST_CASE("Constructor") {
  DLXMatrixNamed M({"A", "B", "C"});
  CHECK(M.nb_items() == 3);
  // clang-format off
  DLXMatrixNamed M1({"A", "B", "C", "D"},
                    { {"rowAB", {"A", "B"}},
                      {"rowAC", {"A", "C"}},
                     {"rowCD", {"C", "D"}} });
  // clang-format on
  CHECK(M1.nb_items() == 4);
  CHECK(M1.nb_opts() == 3);
  CHECK(M1.ith_opt(1) == DLXMatrixNamed::Option({"A", "C"}));
  CHECK(M1.search_iter());
  CHECK(M1.get_solution() == std::vector<std::string>({"rowAB", "rowCD"}));
  CHECK(M1.is_solution({"rowCD", "rowAB"}));
  CHECK(M1.get_opt_ind("rowCD") == 2);
  CHECK(M1.ith_opt(M1.get_opt_ind("rowCD")) ==
        DLXMatrixNamed::Option({"C", "D"}));
  CHECK_THROWS_AS(M1.get_opt_ind("rowXX"), std::runtime_error);
  CHECK_FALSE(M1.search_iter());
  CHECK_THROWS_AS(M1.add_opt("AE", {"A", "E"}), std::out_of_range);
  CHECK_THROWS_WITH_AS(DLXMatrixNamed({"A", "B", "C", "B"}),
                       "DLXMatrixIdent : Duplicate item", std::runtime_error);
}

/////////////////////////////////////////////////////////
TEST_SUITE_END();  // "[dlx_matrix]class DLXMatrixNamed";
/////////////////////////////////////////////////////////

}  // namespace DLX_backtrack
