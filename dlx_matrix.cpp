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
// #ifndef DOCTEST_CONFIG_DISABLE
// #define DOCTEST_CONFIG_IMPLEMENT
// #endif

#include "doctest/doctest.h"

#include "dlx_matrix.hpp"

#include <algorithm>   // sort, transform, random_shuffle
#include <cassert>     // assert
#include <functional>  // bind, equal_to, _2
#include <iostream>    // cout, cin, ...
#include <numeric>     // iota
#include <stdexcept>   // out_of_range
#include <vector>      // vector
#include <experimental/iterator> // ostream_joiner

namespace DLX_backtrack {

std::vector<int> inverse_perm(const std::vector<int> &perm) {
    std::vector<int> inv(perm.size());
    for (size_t i = 0; i < perm.size(); i++)
        inv[perm[i]] = i;
    return inv;
}

TEST_CASE("[dlx_matrix]inverse_perm") {
    CHECK(inverse_perm({}) == std::vector<int>({}));
    CHECK(inverse_perm({0}) == std::vector<int>{0});
    CHECK(inverse_perm({0, 1}) == std::vector<int>{0, 1});
    CHECK(inverse_perm({1, 0}) == std::vector<int>{1, 0});
    CHECK(inverse_perm({1, 0, 3, 2}) == std::vector<int>{1, 0, 3, 2});
    CHECK(inverse_perm({1, 3, 0, 2}) == std::vector<int>{2, 0, 3, 1});
    CHECK(inverse_perm({6, 1, 5, 3, 7, 0, 4, 2}) ==
          std::vector<int>{5, 1, 7, 3, 6, 2, 0, 4});
}

////////////////////////////////////////////////
TEST_SUITE_BEGIN("[dlx_matrix]class DLXMatrix");
////////////////////////////////////////////////

class DLXMatrixFixture {
  public:
    // clang-format off
    DLXMatrixFixture() :
        empty0(0), empty1(1), empty5(5),
        M1_1(1, {{0}}),
        M5_2(5, {{0, 1}, {2, 3, 4}}),
        M5_3(5, {{0, 1}, {2, 3, 4}, {1, 2, 4}}),
        M6_10(6, { {0, 2}, {0, 1}, {1, 4}, {3}, {3, 4}, {5},
                   {1}, {0, 1, 2}, {2, 3, 4}, {1, 4, 5} }),
        TestSample({empty0, empty1, empty5, M1_1, M5_2, M5_3, M6_10})
      {}
    // clang-format on
  protected:
    DLXMatrix empty0, empty1, empty5, M1_1, M5_2, M5_3, M6_10;
    std::vector<DLXMatrix> TestSample;
};

DLXMatrix::DLXMatrix(int nb_col) : heads(nb_col + 1) {
    for (int i = 0; i <= nb_col; i++) {
        heads[i].size = 0;
        heads[i].col_id = i - 1;
        heads[i].node.row_id = -1;
        heads[i].node.head = &heads[i];
        heads[i].node.left = heads[i].node.right = NULL;  // unused
        heads[i].node.up = heads[i].node.down = &heads[i].node;
    }
    heads[nb_col].right = &heads[0];
    for (int i = 0; i < nb_col; i++)
        heads[i].right = &heads[i + 1];
    heads[0].left = &heads[nb_col];
    for (int i = 1; i <= nb_col; i++)
        heads[i].left = &heads[i - 1];
    search_down = true;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "[dlx_matrix]DLXMatrix(int nb_col)") {
    CHECK(empty0.width() == 0);
    CHECK(empty0.height() == 0);
    CHECK(empty5.width() == 5);
    CHECK(empty5.height() == 0);
}

DLXMatrix::DLXMatrix(int nb_col, const std::vector<std::vector<int>> &rows)
    : DLXMatrix(nb_col) {
    for (const auto &r : rows)
        add_row(r);
}
TEST_CASE_FIXTURE(DLXMatrixFixture,
                  "[dlx_matrix]DLXMatrix(int, const vector<vector<int>> &))") {
    SUBCASE("Correct rows") {
        CHECK(M5_2.width() == 5);
        CHECK(M5_2.height() == 2);
        CHECK(M6_10.width() == 6);
        CHECK(M6_10.height() == 10);
    }
    SUBCASE("Row out of bound") {
        CHECK_THROWS_AS(DLXMatrix M(5, {{0, 1}, {2, 3, 5}}), std::out_of_range);
        CHECK_THROWS_AS(DLXMatrix M(3, {{0, 5}}), std::out_of_range);
    }
}

DLXMatrix::DLXMatrix(const DLXMatrix &other) : DLXMatrix(other.width()) {
    for (auto &row : other.rows)
        add_row(row_to_intvector(row));
    for (Node *n : other.work) {
        Node *nd = rows[n->row_id].data() + (n - other.rows[n->row_id].data());
        cover(nd->head);
        choose(nd);
    }
    search_down = other.search_down;
    nb_solutions = other.nb_solutions;
    nb_choices = other.nb_choices;
    nb_dances = other.nb_dances;
}
TEST_CASE_FIXTURE(DLXMatrixFixture,
                  "[dlx_matrix]DLXMatrix(const DLXMatrix &))") {
    for (auto &M : TestSample) {
        CAPTURE(M);
        DLXMatrix N(M);
        CHECK(N.width() == M.width());
        REQUIRE(N.height() == M.height());
        for (size_t i = 0; i < M.height(); i++)
            CHECK(N.int_row(i) == M.int_row(i));
    }
}

DLXMatrix &DLXMatrix::operator=(DLXMatrix other) {
    std::swap(heads, other.heads);
    std::swap(rows, other.rows);
    std::swap(work, other.work);
    search_down = other.search_down;
    nb_solutions = other.nb_solutions;
    nb_choices = other.nb_choices;
    nb_dances = other.nb_dances;
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
            CHECK(N.int_row(i) == M.int_row(i));
    }
}

std::vector<int> DLXMatrix::row_to_intvector(const std::vector<Node> &row) {
    std::vector<int> r;
    r.reserve(row.size());
    std::transform(row.begin(), row.end(), std::back_inserter(r),
                   [](const Node &n) -> int { return n.head->col_id; });
    return r;
}
std::vector<int> DLXMatrix::int_row(size_t i) const {
    return row_to_intvector(rows.at(i));
}
TEST_CASE("method int_row") {
    DLXMatrix M(5, {{0, 1}, {2, 3, 4}, {1, 2, 4}});
    CHECK(M.int_row(0) == std::vector<int>({0, 1}));
    CHECK(M.int_row(1) == std::vector<int>({2, 3, 4}));
    CHECK(M.int_row(2) == std::vector<int>({1, 2, 4}));
}

std::vector<bool>
DLXMatrix::row_to_boolvector(const std::vector<Node> &row) const {
    auto rowint = DLXMatrix::row_to_intvector(row);
    std::sort(rowint.begin(), rowint.end());
    std::vector<bool> res(width(), false);
    auto it = rowint.begin();
    size_t i = 0;
    while (i < width() && it < rowint.end()) {
        if (static_cast<int>(i) == *it) {
            res[i] = true;
            it++;
        }
        i++;
    }
    return res;
}
std::vector<bool> DLXMatrix::bool_row(size_t i) const {
    return row_to_boolvector(rows.at(i));
}
TEST_CASE("method bool_row") {
    DLXMatrix M(5, {{0, 1}, {2, 3, 4}, {1, 2, 4}});
    CHECK(M.bool_row(0) == std::vector<bool>({1, 1, 0, 0, 0}));
    CHECK(M.bool_row(1) == std::vector<bool>({0, 0, 1, 1, 1}));
    CHECK(M.bool_row(2) == std::vector<bool>({0, 1, 1, 0, 1}));
}

void DLXMatrix::check_sizes() const {
    for (Header *h = master()->right; h != master(); h = h->right) {
        int irows = 0;
        for (Node *p = h->node.down; p != &h->node; irows++, p = p->down) {}
        if (h->size != irows)
            throw std::logic_error("wrong size of column");
    }
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method check_sizes") {
    for (auto &M : TestSample) {
        CAPTURE(M);
        CHECK_NOTHROW(M.check_sizes());
    }
}

int DLXMatrix::add_row(const std::vector<int> &r) {
    // Check for bound before modifying anything
    for (auto i : r)
        heads.at(i + 1);

    int row_id = rows.size();
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
TEST_CASE_FIXTURE(DLXMatrixFixture, "method add_row") {
    SUBCASE("Correct row is added") {
        for (auto &M : TestSample) {
            CAPTURE(M);
            DLXMatrix MSave(M);
            if (M.width() >= 4) {
                CHECK_NOTHROW(M.add_row({2, 3}));
                CHECK(M.width() == MSave.width());
                REQUIRE(M.height() == MSave.height() + 1);
                for (size_t i = 0; i < MSave.height(); i++)
                    CHECK(M.int_row(i) == MSave.int_row(i));
                CHECK(M.int_row(MSave.height()) == std::vector<int>({2, 3}));
                CHECK_NOTHROW(M.check_sizes());
            }
        }
    }
    SUBCASE("Check that row is unchanged by failed add of out of bound row") {
        for (auto &M : TestSample) {
            CAPTURE(M);
            DLXMatrix MSave(M);
            if (M.width() < 4) {
                CHECK_THROWS_AS(M.add_row({2, 3}), std::out_of_range);
                CHECK(M.width() == MSave.width());
                REQUIRE(M.height() == MSave.height());
                for (size_t i = 0; i < MSave.height(); i++)
                    CHECK(M.int_row(i) == MSave.int_row(i));
                CHECK_NOTHROW(M.check_sizes());
            }
        }
    }
}

bool DLXMatrix::is_solution(const std::vector<int> &sol) {
    using namespace std::placeholders;
    std::vector<int> cols(width());
    for (int r : sol)
        std::transform(cols.begin(), cols.end(), bool_row(r).begin(),
                       cols.begin(), std::plus<>());
    return std::all_of(cols.begin(), cols.end(),
                       std::bind(std::equal_to<>(), _1, 1));
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
    CHECK_FALSE(M5_2.is_solution({}));
    CHECK_FALSE(M5_2.is_solution({0}));
    CHECK_FALSE(M5_2.is_solution({1}));
    CHECK(M5_2.is_solution({0, 1}));
    CHECK(M5_2.is_solution({1, 0}));
    CHECK_THROWS_AS(M5_2.is_solution({2}), std::out_of_range);

    CHECK(M5_3.is_solution({0, 1}));
    CHECK_THROWS_AS(M5_3.is_solution({1, 3}), std::out_of_range);
    for (const auto &s :
         {std::vector<int>{}, {0}, {1}, {2}, {0, 2}, {1, 2}, {0, 1, 2}}) {
        CAPTURE(s);
        CHECK_FALSE(M5_3.is_solution(s));
    }

    CHECK(M6_10.is_solution({0, 4, 5, 6}));
    CHECK(M6_10.is_solution({6, 0, 5, 4}));
    CHECK_FALSE(M6_10.is_solution({0, 2, 4, 5, 6}));
    CHECK_FALSE(M6_10.is_solution({0, 5, 6}));
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
    int min_size = choice->size;
    for (Header *h = choice->right; h != master(); h = h->right) {
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
std::vector<std::vector<int>> DLXMatrix::search_rec(int maxsol) {
    std::vector<std::vector<int>> res{};
    nb_solutions = nb_choices = nb_dances = 0;
    search_rec_internal(maxsol, res);
    return res;
}
void DLXMatrix::search_rec_internal(int maxsol,
                                    std::vector<std::vector<int>> &res) {
    if (master()->right == master()) {
        nb_solutions++;
        res.push_back(get_solution());
        return;
    }

    Header *choice = choose_min();
    if (choice->size == 0)
        return;

    cover(choice);
    for (Node *row = choice->node.down; row != &choice->node; row = row->down) {
        choose(row);
        search_rec_internal(maxsol, res);
        unchoose(row);
        if (nb_solutions >= maxsol)
            break;
    }
    uncover(choice);
}

std::vector<std::vector<int>>
normalize_solutions(std::vector<std::vector<int>> sols) {
    for (auto &sol : sols)
        std::sort(sol.begin(), sol.end());
    std::sort(sols.begin(), sols.end());
    return sols;
}

TEST_CASE_FIXTURE(DLXMatrixFixture, "method search_rec") {
    CHECK(normalize_solutions(M6_10.search_rec()) ==
          std::vector<std::vector<int>>(
              {{0, 2, 3, 5}, {0, 3, 9}, {0, 4, 5, 6}, {1, 5, 8}, {4, 5, 7}}));
    CHECK(normalize_solutions(M5_2.search_rec()) ==
          std::vector<std::vector<int>>({{0, 1}}));
    CHECK(normalize_solutions(M5_3.search_rec()) ==
          std::vector<std::vector<int>>({{0, 1}}));
    for (auto &M : TestSample) {
        for (auto &s : M.search_rec()) {
            CAPTURE(M);
            CAPTURE(s);
            CHECK(M.is_solution(s));
        }
    }
}

// Knuth dancing links search algorithm
// Iterative version
///////////////////////////////////////
bool DLXMatrix::search_iter() {
    while (search_down || !work.empty()) {
        if (search_down) {  // going down the recursion
            if (master()->right == master()) {
                nb_solutions++;
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
bool DLXMatrix::search_iter(std::vector<int> &v) {
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
            std::vector<std::vector<int>> sols;
            while (M.search_iter())
                sols.push_back(M.get_solution());
            CHECK(sols == N.search_rec());
        }
    }
    SUBCASE("still work after copy") {
        REQUIRE(M6_10.search_iter());
        REQUIRE(M6_10.search_iter());
        DLXMatrix N(M6_10);
        std::vector<std::vector<int>> solM, solN;
        while (M6_10.search_iter())
            solM.push_back(M6_10.get_solution());
        while (N.search_iter())
            solN.push_back(N.get_solution());
        CHECK(solN == solM);
    }
    SUBCASE("still work after assignment") {
        REQUIRE(M6_10.search_iter());
        REQUIRE(M6_10.search_iter());
        DLXMatrix N(0);
        N = M6_10;
        std::vector<std::vector<int>> solM, solN;
        while (M6_10.search_iter())
            solM.push_back(M6_10.get_solution());
        while (N.search_iter())
            solN.push_back(N.get_solution());
        CHECK(solN == solM);
    }
}

std::vector<int> DLXMatrix::get_solution() {
    std::vector<int> r;
    r.reserve(work.size());
    std::transform(work.begin(), work.end(), std::back_inserter(r),
                   [](Node *n) -> int { return n->row_id; });
    return r;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method search_iter") {
    CHECK(M6_10.get_solution() == std::vector<int>({}));
    REQUIRE(M6_10.search_iter());
    CHECK(M6_10.get_solution() == std::vector<int>({5, 0, 2, 3}));
    REQUIRE(M6_10.search_iter());
    CHECK(M6_10.get_solution() == std::vector<int>({5, 0, 6, 4}));
}

void DLXMatrix::reset() {
    nb_solutions = nb_choices = nb_dances = 0;
    while (!work.empty()) {
        Node *row = work.back();
        unchoose(row);
        uncover(row->head);
    }
    search_down = true;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method reset") {
    DLXMatrix N(M6_10);
    REQUIRE(N.search_iter());
    REQUIRE(N.search_iter());
    N.reset();
    std::vector<std::vector<int>> solM, solN;
    while (M6_10.search_iter())
        solM.push_back(M6_10.get_solution());
    while (N.search_iter())
        solN.push_back(N.get_solution());
    CHECK(solN == solM);
}

DLXMatrix DLXMatrix::permuted_inv_columns(const std::vector<int> &perm) {
    assert(perm.size() == width());
    DLXMatrix res(width());
    for (auto &row : rows) {
        std::vector<int> r;
        std::transform(
            row.begin(), row.end(), std::back_inserter(r),
            [&perm](const Node &n) -> int { return perm[n.head->col_id]; });
        res.add_row(r);
    }
    return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method permuted_inv_columns") {
    SUBCASE("empty0") {
        std::vector<int> perm{};
        CHECK(empty0.permuted_inv_columns(perm).width() == 0);
        CHECK(empty0.permuted_inv_columns(perm).height() == 0);
    }
    SUBCASE("empty5") {
        std::vector<int> perm{3,4,0,2,1};
        CHECK(empty5.permuted_inv_columns(perm).width() == 5);
        CHECK(empty5.permuted_inv_columns(perm).height() == 0);
    }
    SUBCASE("M6_10") {
        std::vector<int> perm{4,3,2,0,5,1};
        DLXMatrix M = M6_10.permuted_inv_columns(perm);
        CHECK(M.width() == M6_10.width());
        CHECK(M.height() == M6_10.height());
        CHECK(normalize_solutions(M.search_rec()) ==
              normalize_solutions(M6_10.search_rec()));
        for (size_t r = 0; r < M6_10.height(); r++) {
            for (size_t c = 0; r < M6_10.width(); r++) {
                CHECK(M.bool_row(r)[perm[c]] == M6_10.bool_row(r)[c]);
            }
        }
    }
}
DLXMatrix DLXMatrix::permuted_columns(const std::vector<int> &perm) {
    return permuted_inv_columns(inverse_perm(perm));
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method permuted_columns") {
    SUBCASE("empty0") {
        std::vector<int> perm{};
        CHECK(empty0.permuted_columns(perm).width() == 0);
        CHECK(empty0.permuted_columns(perm).height() == 0);
    }
    SUBCASE("empty5") {
        std::vector<int> perm{3,4,0,2,1};
        CHECK(empty5.permuted_columns(perm).width() == 5);
        CHECK(empty5.permuted_columns(perm).height() == 0);
    }
    SUBCASE("M6_10") {
        std::vector<int> perm{4,3,2,0,5,1};
        DLXMatrix M = M6_10.permuted_columns(perm);
        CHECK(M.width() == M6_10.width());
        CHECK(M.height() == M6_10.height());
        CHECK(normalize_solutions(M.search_rec()) ==
              normalize_solutions(M6_10.search_rec()));
        for (size_t r = 0; r < M6_10.height(); r++) {
            for (size_t c = 0; r < M6_10.width(); r++) {
                CHECK(M.bool_row(r)[c] == M6_10.bool_row(r)[perm[c]]);
            }
        }
    }
}

DLXMatrix DLXMatrix::permuted_rows(const std::vector<int> &perm) {
    assert(perm.size() == height());
    DLXMatrix res(width());
    for (int i : perm)
        res.add_row(row_to_intvector(rows[i]));
    return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method permuted_rows") {
    SUBCASE("empty") {
        std::vector<int> perm{};
        CHECK(empty0.permuted_rows(perm).width() == 0);
        CHECK(empty0.permuted_rows(perm).height() == 0);
        CHECK(empty5.permuted_rows(perm).width() == 5);
        CHECK(empty5.permuted_rows(perm).height() == 0);
    }
    SUBCASE("M6_10") {
        std::vector<int> perm{4,7,3,8,2,0,5,1,9,6};
        DLXMatrix M = M6_10.permuted_rows(perm);
        CHECK(M.width() == M6_10.width());
        CHECK(M.height() == M6_10.height());
        for (int i=0; i<10; i++) {
            CAPTURE(i);
            CHECK(M.int_row(i) == M6_10.int_row(perm[i]));
        }
    }
}

bool DLXMatrix::search_random(std::vector<int> &sol) {
    std::vector<int> row_perm(height());
    std::iota(row_perm.begin(), row_perm.end(), 0);
    std::random_shuffle(row_perm.begin(), row_perm.end());

    std::vector<int> col_perm(width());
    std::iota(col_perm.begin(), col_perm.end(), 0);
    std::random_shuffle(col_perm.begin(), col_perm.end());

    DLXMatrix M = permuted_columns(col_perm).permuted_rows(row_perm);
    bool res;
    if ((res = M.search_iter())) {
        std::vector<int> v = M.get_solution();
        sol.resize(v.size());
        for (size_t i = 0; i < v.size(); i++)
            sol[i] = row_perm[v[i]];
    }
    return res;
}
TEST_CASE_FIXTURE(DLXMatrixFixture, "method search_random") {
    for (int i=0; i<20; i++) {
        std::vector<int> sol;
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
        out << M.row_to_boolvector(row) << "\n";
    return out;
}

TEST_SUITE_END();  // "[dlx_matrix]class DLXMatrix";

}  // namespace DLX_backtrack


namespace doctest {

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v) {
    return DLX_backtrack::operator<<(out, v);
}

}  // namespace doctest
