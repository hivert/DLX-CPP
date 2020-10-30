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
#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#ifndef DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_DISABLE
#endif
#endif

#include "doctest/doctest.h"

#include "dlx_matrix.hpp"

#include <algorithm>   // sort, transform, shuffle
#include <functional>  // bind, equal_to, _2
#include <iostream>    // cout, cin, ...
#include <numeric>     // iota
#include <random>      // default_random_engine
#include <stdexcept>   // out_of_range
#include <vector>      // vector

namespace DLX_backtrack {

using Vect1D = DLXMatrix::Vect1D;
using Vect2D = DLXMatrix::Vect2D;

///////////////////////////////////////
TEST_SUITE_BEGIN("[dlx_matrix]Errors");
///////////////////////////////////////

void check_size(const std::string &s, size_t expected, size_t sz) {
    if (sz != expected)
        throw size_mismatch_error(s, expected, sz);
}
void check_bound(const std::string &s, size_t bound, size_t i) {
    if (i > bound)
        throw out_of_bound_error(s, bound, i);
}

TEST_CASE("[dlx_matrix]size_mismatch_error") {
    CHECK_THROWS_AS(throw size_mismatch_error("bla", 2, 3),
                    size_mismatch_error);
}
TEST_CASE("[dlx_matrix]check_size") {
    CHECK_NOTHROW(check_size("bla", 2, 2));
    CHECK_THROWS_AS(check_size("bla", 2, 3), size_mismatch_error);
}

TEST_CASE("[dlx_matrix]out_of_bound_error") {
    CHECK_THROWS_AS(throw out_of_bound_error("bla", 2, 3), out_of_bound_error);
}
TEST_CASE("[dlx_matrix]check_bound") {
    CHECK_NOTHROW(check_bound("bla", 2, 2));
    CHECK_NOTHROW(check_bound("bla", 3, 2));
    CHECK_NOTHROW(check_bound("bla", 3, 0));
    CHECK_THROWS_AS(check_bound("bla", 2, 3), out_of_bound_error);
}

///////////////////////////////////////////
TEST_SUITE_END();  // [dlx_matrix]Errors");
///////////////////////////////////////////

Vect1D inverse_perm(const Vect1D &perm) {
    Vect1D inv(perm.size());
    for (size_t i = 0; i < perm.size(); i++)
        inv[perm[i]] = i;
    return inv;
}

TEST_CASE("[dlx_matrix]inverse_perm") {
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
// 11 [0 0 0 0 0 0 0 0 0 1]  // Secondary column not coded
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
    std::vector<DLXMatrix> TestSample;
};

DLXMatrix::DLXMatrix(size_t nb_col, size_t sec_start)
    : nb_primary(sec_start), heads(nb_col + 1), search_down(true),
      nb_choices(0), nb_dances(0) {
    check_bound("nb_primary", nb_col, nb_primary);
    for (size_t i = 0; i <= nb_col; i++) {
        heads[i].size = 0;
        heads[i].node.up = heads[i].node.down = &heads[i].node;
        // heads[i].node.head = &heads[i];  // unused
        // heads[i].node.row_id = -1  // unused;
        // heads[i].node.left = heads[i].node.right = nullptr;  // unused
    }
    heads[nb_col].right = &heads[0];
    for (size_t i = 0; i < nb_col; i++)
        heads[i].right = &heads[i + 1];
    heads[0].left = &heads[nb_col];
    heads[0].col_id =  // large enough sentinel value for choose_min
        std::numeric_limits<size_t>::max();
    for (size_t i = 1; i <= nb_col; i++) {
        heads[i].col_id = i - 1;
        heads[i].left = &heads[i - 1];
    }
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "[dlx_matrix]DLXMatrix(size_t)") {
    CHECK(empty0.width() == 0);
    CHECK(empty0.height() == 0);
    CHECK(empty5.width() == 5);
    CHECK(empty5.height() == 0);
}
TEST_CASE("[dlx_matrix]DLXMatrix(size_t, size_t)") {
    CHECK_NOTHROW(DLXMatrix(0, 0));
    CHECK_THROWS_AS(DLXMatrix(0, 1), out_of_bound_error);
    CHECK_NOTHROW(DLXMatrix(5, 0));
    CHECK_NOTHROW(DLXMatrix(5, 2));
    CHECK_NOTHROW(DLXMatrix(5, 5));
    CHECK_THROWS_AS(DLXMatrix(5, 6), out_of_bound_error);
}

DLXMatrix::DLXMatrix(size_t nb_col, size_t sec_start, const Vect2D &rows)
    : DLXMatrix(nb_col, sec_start) {
    for (const auto &r : rows)
        add_row_sparse(r);
}
TEST_CASE_FIXTURE(
    DLXMatrixFixture,
    "[dlx_matrix]DLXMatrix(size_t, const Vect2D &))") {
    SUBCASE("Correct rows") {
        CHECK(M5_2.width() == 5);
        CHECK(M5_2.height() == 2);
        CHECK(M6_10.width() == 6);
        CHECK(M6_10.height() == 10);
    }
    SUBCASE("Row out of range") {
        CHECK_THROWS_AS(DLXMatrix(5, {{0, 1}, {2, 3, 5}}), std::out_of_range);
        CHECK_THROWS_AS(DLXMatrix(3, {{0, 5}}), std::out_of_range);
    }
}
TEST_CASE(
    "[dlx_matrix]DLXMatrix(size_t, size_t, const Vect2D &))") {
    SUBCASE("Correct call") {
        CHECK_NOTHROW(DLXMatrix(0, 0, {}));
        CHECK_NOTHROW(DLXMatrix(1, 1, {{0}}));
        CHECK_NOTHROW(DLXMatrix(3, 2, {{0}, {1, 2}}));
    }
    SUBCASE("nb_primary out of bound") {
        CHECK_THROWS_AS(DLXMatrix(0, 1, {}), out_of_bound_error);
    }
    SUBCASE("Row out of range") {
        CHECK_THROWS_AS(DLXMatrix(5, 4, {{0, 1}, {2, 3, 5}}),
                        std::out_of_range);
        CHECK_THROWS_AS(DLXMatrix(3, 3, {{0, 5}}), std::out_of_range);
    }
}

DLXMatrix::DLXMatrix(const DLXMatrix &other)
    : DLXMatrix(other.width(), other.nb_primary) {
    for (auto &row : other.rows)
        add_row_sparse(row_sparse(row));
    for (Node *n : other.work) {
        Node *nd = &rows[n->row_id][n - other.rows[n->row_id].data()];
        cover(nd->head);
        choose(nd);
    }
    search_down = other.search_down;
    nb_choices = other.nb_choices;
    nb_dances = other.nb_dances;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "[dlx_matrix]DLXMatrix copy constructor") {
    for (auto &M : TestSample) {
        CAPTURE(M);
        DLXMatrix N(M);
        CHECK(N.width() == M.width());
        REQUIRE(N.height() == M.height());
        for (size_t i = 0; i < M.height(); i++)
            CHECK(N.row_sparse(i) == M.row_sparse(i));
    }
}
TEST_CASE_FIXTURE(DLXMatrixFixture,
                  "[dlx_matrix]DLXMatrix move constructor/assignement") {
    DLXMatrix M = std::move(M5_3);
    CHECK(M.width() == 5);

    M = std::move(M6_10);
    CHECK(M.width() == 6);

    std::swap(empty0, empty5);
    CHECK(empty0.width() == 5);
    CHECK(empty5.width() == 0);
}

DLXMatrix &DLXMatrix::operator=(const DLXMatrix &other) {
    DLXMatrix res(other);  // Construct a correct copy
    nb_primary = res.nb_primary;
    heads = std::move(res.heads);
    rows = std::move(res.rows);
    work = std::move(res.work);
    search_down = res.search_down;
    nb_choices = res.nb_choices;
    nb_dances = res.nb_dances;
    return *this;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "operator=") {
    for (auto &M : TestSample) {
        CAPTURE(M);
        DLXMatrix N(0);
        N = M;
        CHECK(N.width() == M.width());
        REQUIRE(N.height() == M.height());
        for (size_t i = 0; i < M.height(); i++)
            CHECK(N.row_sparse(i) == M.row_sparse(i));
    }
}

Vect1D DLXMatrix::row_sparse(const std::vector<Node> &row) {
    Vect1D r;
    r.reserve(row.size());
    std::transform(row.begin(), row.end(), std::back_inserter(r),
                   [](const Node &n) -> size_t { return n.head->col_id; });
    return r;
}
Vect1D DLXMatrix::row_sparse(size_t i) const {
    return row_sparse(rows.at(i));
}
TEST_CASE("method row_sparse") {
    DLXMatrix M(5, {{0, 1}, {2, 3, 4}, {1, 2, 4}});
    CHECK(M.row_sparse(0) == Vect1D({0, 1}));
    CHECK(M.row_sparse(1) == Vect1D({2, 3, 4}));
    CHECK(M.row_sparse(2) == Vect1D({1, 2, 4}));
}

std::vector<bool> DLXMatrix::row_dense(const std::vector<Node> &row) const {
    return row_to_dense(DLXMatrix::row_sparse(row));
}
std::vector<bool> DLXMatrix::row_dense(size_t i) const {
    return row_dense(rows.at(i));
}
TEST_CASE("method row_dense") {
    DLXMatrix M(5, {{0, 1}, {2, 3, 4}, {1, 2, 4}});
    CHECK(M.row_dense(0) == std::vector<bool>({1, 1, 0, 0, 0}));
    CHECK(M.row_dense(1) == std::vector<bool>({0, 0, 1, 1, 1}));
    CHECK(M.row_dense(2) == std::vector<bool>({0, 1, 1, 0, 1}));
}

void DLXMatrix::check_sizes() const {
    for (Header *h = master()->right; h != master(); h = h->right) {
        size_t irows = 0;
        for (Node *p = h->node.down; p != &h->node; irows++, p = p->down) {}
        check_size("column", h->size, irows);
    }
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method check_sizes") {
    for (auto &M : TestSample) {
        CAPTURE(M);
        CHECK_NOTHROW(M.check_sizes());
    }
}

size_t DLXMatrix::add_row_sparse(const Vect1D &r) {
    // Check for bound before modifying anything
    for (auto i : r)
        heads.at(i + 1);

    size_t row_id = rows.size();
    rows.emplace_back(r.size());
    std::vector<Node> &row = rows.back();

    for (size_t i = 0; i < r.size(); i++) {
        auto &h = heads[r[i] + 1];
        row[i].row_id = row_id;
        row[i].head = &h;
        h.size++;
        row[i].down = &h.node;
        row[i].up = h.node.up;
        row[i].up->down = h.node.up = &row[i];
    }
    row.back().right = &row[0];
    for (size_t i = 0; i < r.size() - 1; i++)
        row[i].right = &row[i + 1];
    row[0].left = &row.back();
    for (size_t i = 1; i < r.size(); i++)
        row[i].left = &row[i - 1];
    return row_id;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method add_row_sparse") {
    SUBCASE("Correct row is added") {
        for (auto &M : TestSample) {
            CAPTURE(M);
            DLXMatrix MSave(M);
            if (M.width() >= 4) {
                size_t rid;
                CHECK_NOTHROW((rid = M.add_row_sparse({2, 3})));
                CHECK(rid == MSave.height());
                CHECK(M.width() == MSave.width());
                REQUIRE(M.height() == MSave.height() + 1);
                for (size_t i = 0; i < MSave.height(); i++)
                    CHECK(M.row_sparse(i) == MSave.row_sparse(i));
                CHECK(M.row_sparse(MSave.height()) == Vect1D({2, 3}));
                CHECK_NOTHROW(M.check_sizes());
            }
        }
    }
    SUBCASE("Check that row is unchanged by failed add of out of bound row") {
        for (auto &M : TestSample) {
            CAPTURE(M);
            DLXMatrix MSave(M);
            if (M.width() < 4) {
                CHECK_THROWS_AS(M.add_row_sparse({2, 3}), std::out_of_range);
                CHECK(M.width() == MSave.width());
                REQUIRE(M.height() == MSave.height());
                for (size_t i = 0; i < MSave.height(); i++)
                    CHECK(M.row_sparse(i) == MSave.row_sparse(i));
                CHECK_NOTHROW(M.check_sizes());
            }
        }
    }
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method add_row") {
    CHECK(M5_3.add_row({2, 3}) == 3);
    CHECK(M5_3.row_sparse(3) == Vect1D({2, 3}));
}

Vect1D DLXMatrix::row_to_sparse(const std::vector<bool> &row) const {
    check_size("row", row.size(), width());
    Vect1D res;
    for (size_t i = 0; i < row.size(); i++) {
        if (row[i])
            res.push_back(i);
    }
    return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method row_to_sparse") {
    CHECK(M5_3.row_to_sparse({0, 1, 1, 0, 0}) == Vect1D({1, 2}));
    CHECK(M5_3.row_to_sparse({0, 1, 0, 1, 1}) == Vect1D({1, 3, 4}));
    CHECK_THROWS_AS(M5_3.row_to_sparse({0, 1, 1, 0, 1, 1}),
                    size_mismatch_error);
    CHECK_THROWS_AS(M5_3.row_to_sparse({0, 1, 1, 0}), size_mismatch_error);
}

std::vector<bool> DLXMatrix::row_to_dense(Vect1D row) const {
    // Check for bound
    for (auto i : row)
        heads.at(i + 1);
    std::sort(row.begin(), row.end());
    std::vector<bool> res(width(), false);
    auto it = row.begin();
    size_t i = 0;
    while (i < width() && it < row.end()) {
        if (i == *it) {
            res[i] = true;
            it++;
        }
        i++;
    }
    return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method row_to_dense") {
    CHECK(M5_3.row_to_dense({1, 2}) == std::vector<bool>({0, 1, 1, 0, 0}));
    CHECK(M5_3.row_to_dense({1, 3, 4}) == std::vector<bool>({0, 1, 0, 1, 1}));
    CHECK_THROWS_AS(M5_3.row_to_dense({1, 3, 5}), std::out_of_range);
}

size_t DLXMatrix::add_row_dense(const std::vector<bool> &r) {
    return add_row_sparse(row_to_sparse(r));
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method add_row_dense") {
    CHECK(M5_3.add_row_dense({0, 0, 1, 1, 0}) == 3);
    CHECK(M5_3.row_sparse(3) == Vect1D({2, 3}));
    CHECK_THROWS_AS(M5_3.add_row_dense({0, 1, 1, 0}), size_mismatch_error);
    CHECK_THROWS_AS(M5_3.add_row_dense({0, 1, 1, 0, 1, 0}),
                    size_mismatch_error);
}

bool DLXMatrix::is_solution(const Vect1D &sol) {
    using std::placeholders::_1;
    Vect1D cols(width());
    for (size_t r : sol) {
        std::transform(cols.begin(), cols.end(), row_dense(r).begin(),
                       cols.begin(), std::plus<>());
    }
    for (size_t i = 0; i < nb_primary; i++)
        if (cols[i] != 1)
            return false;
    for (size_t i = nb_primary; i < width(); i++)
        if (cols[i] > 1)
            return false;
    return true;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method is_solution") {
    SUBCASE("Empty 0x0 Matrix") {
        CHECK(empty0.is_solution({}));
        CHECK_THROWS_AS(empty0.is_solution({0}), std::out_of_range);
    }
    SUBCASE("Empty 0x5 Matrix") {
        CHECK_FALSE(empty5.is_solution({}));
        CHECK_THROWS_AS(empty5.is_solution({0}), std::out_of_range);
    }
    SUBCASE("M5_2 Matrix") {
        CHECK_FALSE(M5_2.is_solution({}));
        CHECK_FALSE(M5_2.is_solution({0}));
        CHECK_FALSE(M5_2.is_solution({1}));
        CHECK(M5_2.is_solution({0, 1}));
        CHECK(M5_2.is_solution({1, 0}));
        CHECK_THROWS_AS(M5_2.is_solution({2}), std::out_of_range);
    }
    SUBCASE("M5_3 Matrix") {
        CHECK(M5_3.is_solution({0, 1}));
        CHECK_THROWS_AS(M5_3.is_solution({1, 3}), std::out_of_range);
        for (const auto &s :
             {Vect1D{}, {0}, {1}, {2}, {0, 2}, {1, 2}, {0, 1, 2}}) {
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

void DLXMatrix::cover(Header *col) {
    col->left->right = col->right;
    col->right->left = col->left;

    for (Node *row = col->node.down; row != &col->node; row = row->down) {
        for (Node *nr = row->right; nr != row; nr = nr->right) {
            nr->up->down = nr->down;
            nr->down->up = nr->up;
            nr->head->size--;
            nb_dances++;
        }
    }
}
void DLXMatrix::choose(Node *row) {
    nb_choices++;
    work.push_back(row);
    for (Node *nr = row->right; nr != row; nr = nr->right)
        cover(nr->head);
}

void DLXMatrix::uncover(Header *col) {
    col->left->right = col;
    col->right->left = col;

    for (Node *row = col->node.up; row != &col->node; row = row->up) {
        for (Node *nr = row->left; nr != row; nr = nr->left) {
            nr->head->size++;
            nr->up->down = nr;
            nr->down->up = nr;
        }
    }
}
void DLXMatrix::unchoose(Node *row) {
    for (Node *nr = row->left; nr != row; nr = nr->left)
        uncover(nr->head);
    work.pop_back();
}

DLXMatrix::Header *DLXMatrix::choose_min() {
    Header *choice = master()->right;
    size_t min_size = choice->size;
    for (Header *h = choice->right; h->col_id < nb_primary; h = h->right) {
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
    if (master()->right->col_id >= nb_primary) {
        res.push_back(get_solution());
        return;
    }

    Header *choice = choose_min();
    if (choice->size == 0)
        return;

    cover(choice);
    for (Node *row = choice->node.down; row != &choice->node; row = row->down) {
        choose(row);
        search_rec_internal(max_sol, res);
        unchoose(row);
        if (res.size() >= max_sol)
            break;
    }
    uncover(choice);
}

DLXMatrix::Vect2D normalize_solutions(DLXMatrix::Vect2D sols) {
    for (auto &sol : sols)
        std::sort(sol.begin(), sol.end());
    std::sort(sols.begin(), sols.end());
    return sols;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method search_rec") {
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
        CHECK(normalize_solutions(MA2AB_8.search_rec()) ==
              Vect2D({{0, 4, 8, 9},
                      {0, 5, 7, 9},
                      {0, 8, 9, 10},
                      {1, 3, 8, 9},
                      {2, 3, 7, 9}}));
    }

    SUBCASE("Check that all found solutions are actual solutions") {
        for (auto &M : TestSample) {
            for (auto &s : M.search_rec()) {
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
    while (search_down || !work.empty()) {
        if (search_down) {  // going down the recursion
            if (master()->right->col_id >= nb_primary) {
                search_down = false;
                return true;
            }
            Header *choice = choose_min();
            if (choice->size == 0) {
                search_down = false;
            } else {
                cover(choice);
                choose(choice->node.down);
            }
        } else {  // going up the recursion
            Node *row = work.back();
            Header *choice = row->head;
            unchoose(row);
            row = row->down;
            if (row != &choice->node) {
                choose(row);
                search_down = true;
            } else {
                uncover(choice);
            }
        }
    }
    return false;
}
bool DLXMatrix::search_iter(Vect1D &v) {
    bool res;
    if ((res = search_iter()))
        v = get_solution();
    return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method search_iter") {
    SUBCASE("agrees with search_rec") {
        for (auto M : TestSample) {
            CAPTURE(M);
            DLXMatrix N(M);
            DLXMatrix::Vect2D sols;
            while (M.search_iter())
                sols.push_back(M.get_solution());
            CHECK(sols == N.search_rec());
        }
    }
    SUBCASE("still work after copy") {
        REQUIRE(M6_10.search_iter());  // consumes one solution
        REQUIRE(M6_10.search_iter());  // consumes one solution
        DLXMatrix N(M6_10);
        Vect2D solM, solN;
        while (M6_10.search_iter())
            solM.push_back(M6_10.get_solution());
        while (N.search_iter())
            solN.push_back(N.get_solution());
        CHECK(solN == solM);
    }
    SUBCASE("still work after assignment") {
        REQUIRE(M6_10.search_iter());  // consumes one solution
        REQUIRE(M6_10.search_iter());  // consumes one solution
        DLXMatrix N(0);
        N = M6_10;
        Vect2D solM, solN;
        while (M6_10.search_iter())
            solM.push_back(M6_10.get_solution());
        while (N.search_iter())
            solN.push_back(N.get_solution());
        CHECK(solN == solM);
    }
}

Vect1D DLXMatrix::get_solution() {
    Vect1D r;
    r.reserve(work.size());
    std::transform(work.begin(), work.end(), std::back_inserter(r),
                   [](Node *n) -> size_t { return n->row_id; });
    return r;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method get_solution") {
    CHECK(M6_10.get_solution() == Vect1D({}));
    REQUIRE(M6_10.search_iter());
    CHECK(M6_10.get_solution() == Vect1D({5, 0, 2, 3}));
    REQUIRE(M6_10.search_iter());
    CHECK(M6_10.get_solution() == Vect1D({5, 0, 6, 4}));
}

void DLXMatrix::reset() {
    nb_choices = nb_dances = 0;
    while (!work.empty()) {
        Node *row = work.back();
        unchoose(row);
        uncover(row->head);
    }
    search_down = true;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method reset") {
    DLXMatrix N(M6_10);
    REQUIRE(N.search_iter());  // consumes one solution
    REQUIRE(N.search_iter());  // consumes one solution
    N.reset();
    Vect2D solM, solN;
    while (M6_10.search_iter())
        solM.push_back(M6_10.get_solution());
    while (N.search_iter())
        solN.push_back(N.get_solution());
    CHECK(solN == solM);
}

DLXMatrix DLXMatrix::permuted_inv_columns(const Vect1D &perm) {
    check_size("permutation", perm.size(), width());
    DLXMatrix res(width());
    for (auto &row : rows) {
        Vect1D r;
        std::transform(
            row.begin(), row.end(), std::back_inserter(r),
            [&perm](const Node &n) -> size_t { return perm[n.head->col_id]; });
        res.add_row_sparse(r);
    }
    return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method permuted_inv_columns") {
    SUBCASE("empty0") {
        Vect1D perm{};
        CHECK(empty0.permuted_inv_columns(perm).width() == 0);
        CHECK(empty0.permuted_inv_columns(perm).height() == 0);
    }
    SUBCASE("empty5") {
        Vect1D perm{3, 4, 0, 2, 1};
        CHECK(empty5.permuted_inv_columns(perm).width() == 5);
        CHECK(empty5.permuted_inv_columns(perm).height() == 0);
    }
    SUBCASE("M6_10") {
        Vect1D perm{4, 3, 2, 0, 5, 1};
        DLXMatrix M = M6_10.permuted_inv_columns(perm);
        CHECK(M.width() == M6_10.width());
        CHECK(M.height() == M6_10.height());
        CHECK(normalize_solutions(M.search_rec()) ==
              normalize_solutions(M6_10.search_rec()));
        for (size_t r = 0; r < M6_10.height(); r++) {
            for (size_t c = 0; c < M6_10.width(); c++) {
                CHECK(M.row_dense(r)[perm[c]] == M6_10.row_dense(r)[c]);
            }
        }
    }
}
DLXMatrix DLXMatrix::permuted_columns(const Vect1D &perm) {
    return permuted_inv_columns(inverse_perm(perm));
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method permuted_columns") {
    SUBCASE("empty0") {
        Vect1D perm{};
        CHECK(empty0.permuted_columns(perm).width() == 0);
        CHECK(empty0.permuted_columns(perm).height() == 0);
    }
    SUBCASE("empty5") {
        Vect1D perm{3, 4, 0, 2, 1};
        CHECK(empty5.permuted_columns(perm).width() == 5);
        CHECK(empty5.permuted_columns(perm).height() == 0);
    }
    SUBCASE("M6_10") {
        Vect1D perm{4, 3, 2, 0, 5, 1};
        DLXMatrix M = M6_10.permuted_columns(perm);
        CHECK(M.width() == M6_10.width());
        CHECK(M.height() == M6_10.height());
        CHECK(normalize_solutions(M.search_rec()) ==
              normalize_solutions(M6_10.search_rec()));
        for (size_t r = 0; r < M6_10.height(); r++) {
            for (size_t c = 0; c < M6_10.width(); c++) {
                CHECK(M.row_dense(r)[c] == M6_10.row_dense(r)[perm[c]]);
            }
        }
    }
}

DLXMatrix DLXMatrix::permuted_rows(const Vect1D &perm) {
    if (perm.size() != height())
        throw size_mismatch_error("permutation", perm.size(), height());
    DLXMatrix res(width());
    for (size_t i : perm)
        res.add_row_sparse(row_sparse(rows[i]));
    return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method permuted_rows") {
    SUBCASE("empty") {
        Vect1D perm{};
        CHECK(empty0.permuted_rows(perm).width() == 0);
        CHECK(empty0.permuted_rows(perm).height() == 0);
        CHECK(empty5.permuted_rows(perm).width() == 5);
        CHECK(empty5.permuted_rows(perm).height() == 0);
    }
    SUBCASE("M6_10") {
        Vect1D perm{4, 7, 3, 8, 2, 0, 5, 1, 9, 6};
        DLXMatrix M = M6_10.permuted_rows(perm);
        CHECK(M.width() == M6_10.width());
        CHECK(M.height() == M6_10.height());
        for (size_t i = 0; i < 10; i++) {
            CAPTURE(i);
            CHECK(M.row_sparse(i) == M6_10.row_sparse(perm[i]));
        }
    }
}

bool DLXMatrix::search_random(Vect1D &sol) {
    std::random_device rd{};
    std::default_random_engine rng{rd()};

    Vect1D row_perm(height());
    std::iota(row_perm.begin(), row_perm.end(), 0);
    std::shuffle(row_perm.begin(), row_perm.end(), rng);

    Vect1D col_perm(width());
    std::iota(col_perm.begin(), col_perm.end(), 0);
    std::shuffle(col_perm.begin(), col_perm.end(), rng);

    DLXMatrix M = permuted_columns(col_perm).permuted_rows(row_perm);
    bool res;
    if ((res = M.search_iter())) {
        Vect1D v = M.get_solution();
        sol.resize(v.size());
        for (size_t i = 0; i < v.size(); i++)
            sol[i] = row_perm[v[i]];
    }
    return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method search_random") {
    for (size_t i = 0; i < 20; i++) {
        Vect1D sol;
        REQUIRE(M6_10.search_random(sol));
        CAPTURE(sol);
        CHECK(M6_10.is_solution(sol));
    }
}

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v) {
    out << '[';
    auto it = v.begin(), end = v.end();
    if (it != end) {
        out << *it;
        for (++it; it != end; ++it)
            out << ", " << *it;
    }
    out << "]";
    return out;
}

std::ostream &operator<<(std::ostream &out, const DLXMatrix &M) {
    for (auto &row : M.rows)
        out << M.row_dense(row) << "\n";
    return out;
}

////////////////////////////////////////////////////
TEST_SUITE_END();  // "[dlx_matrix]class DLXMatrix";
////////////////////////////////////////////////////

}  // namespace DLX_backtrack

namespace doctest {

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v) {
    return DLX_backtrack::operator<<(out, v);
}

}  // namespace doctest
