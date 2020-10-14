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
#include <algorithm>  // For sort, transform, random_shuffle
#include <cassert>
#include <iostream>
#include <numeric>  // For iota
#include <stdexcept>
#include <vector>


namespace DLX_backtrack {

std::vector<int> inverse_perm(const std::vector<int> &perm) {
    std::vector<int> inv(perm.size());
    for (size_t i = 0; i < perm.size(); i++)
        inv[perm[i]] = i;
    return inv;
}

TEST_CASE("[dance]inverse_perm") {
    CHECK(inverse_perm({}) == std::vector<int>{});
    CHECK(inverse_perm({0}) == std::vector<int>{0});
    CHECK(inverse_perm({0, 1}) == std::vector<int>{0, 1});
    CHECK(inverse_perm({1, 0}) == std::vector<int>{1, 0});
    CHECK(inverse_perm({1, 0, 3, 2}) == std::vector<int>{1, 0, 3, 2});
    CHECK(inverse_perm({1, 3, 0, 2}) == std::vector<int>{2, 0, 3, 1});
    CHECK(inverse_perm({6, 1, 5, 3, 7, 0, 4, 2}) ==
          std::vector<int>{5, 1, 7, 3, 6, 2, 0, 4});
}



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
TEST_CASE("[dance]DLXMatrix::DLXMatrix(int nb_col)") {
    CHECK(DLXMatrix(5).width() == 5);
    CHECK(DLXMatrix(5).height() == 0);
}

DLXMatrix::DLXMatrix(int nb_col, const std::vector<std::vector<int> > &rows)
    : DLXMatrix(nb_col) {
    for (const auto &r : rows)
        add_row(r);
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


std::vector<int> DLXMatrix::row_to_intvector(const std::vector<Node> &row) {
    std::vector<int> r;
    r.reserve(row.size());
    std::transform(row.begin(), row.end(), std::back_inserter(r),
                   [](const Node &n) -> int { return n.head->col_id; });
    return r;
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

std::vector<int> DLXMatrix::int_row(size_t i) const {
    return row_to_intvector(rows.at(i));
}
std::vector<bool> DLXMatrix::bool_row(size_t i) const  {
    return row_to_boolvector(rows.at(i));
}


std::vector<int> DLXMatrix::get_solution() {
    std::vector<int> r;
    r.reserve(work.size());
    std::transform(work.begin(), work.end(), std::back_inserter(r),
                   [](Node *n) -> int { return n->row_id; });
    return r;
}

void DLXMatrix::print_columns() const {
    for (Header *h = master()->right; h != master(); h = h->right) {
        std::cout << h->col_id << "(" << h->size << ") ";
    }
    std::cout << std::endl;
}

void DLXMatrix::check_sizes() const {
    std::cout << "sizes: [ ";
    for (Header *h = master()->right; h != master(); h = h->right) {
        int irows = 0;
        for (Node *p = h->node.down; p != &h->node; irows++, p = p->down) {}
        if (h->size != irows)
            throw std::logic_error("wrong size of column");
        std::cout << h->col_id << "(" << irows << ") ";
    }
    std::cout << "]\n";
}

void DLXMatrix::print_solution(const std::vector<Node *> &solution) const {
    std::cout << "Solution: (size = " << work.size() << ") : \n";
    for (const Node *n : solution) {
        std::cout << n->row_id << " ";
        // std::cout << " " << n->row_id << " :";
        // for (const Node &i : rows[n->row_id])
        //     std::cout << " " << i.head->col_id;
        // std::cout << "\n";
    }
    std::cout << "End" << std::endl;
}

int DLXMatrix::add_row(const std::vector<int> &r) {
    int row_id = rows.size();
    rows.push_back(std::vector<Node>(r.size()));
    std::vector<Node> &row = rows.back();

    for (size_t i = 0; i < r.size(); i++) {
        auto &h = heads.at(r[i] + 1);
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
std::vector<std::vector<int> > DLXMatrix::search_rec(int maxsol) {
    std::vector<std::vector<int> > res{};
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

void DLXMatrix::reset() {
    while (!work.empty()) {
        Node *row = work.back();
        unchoose(row);
        uncover(row->head);
    }
    search_down = true;
}

// Knuth dancing links search algorithm
// Iterative version
///////////////////////////////////////
bool DLXMatrix::search_iter() {
    nb_solutions = nb_choices = nb_dances = 0;
    do {
        while (search_down) {  // going down the recursion
            if (master()->right == master()) {
                nb_solutions++;
                search_down = false;
                return true;
            }
            Header *choice = choose_min();
            if (choice->size == 0) {
                work.pop_back();
                search_down = false;
                break;
            }
            cover(choice);
            choose(choice->node.down);
        }
        while (!search_down && !work.empty()) {  // going up the recursion
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
    } while (!work.empty());
    return false;
}
bool DLXMatrix::search_iter(std::vector<int> &v) {
    bool res;
    if ((res = search_iter()))
        v = get_solution();
    return res;
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
DLXMatrix DLXMatrix::permuted_columns(const std::vector<int> &perm) {
    return permuted_inv_columns(inverse_perm(perm));
}
DLXMatrix DLXMatrix::permuted_rows(const std::vector<int> &perm) {
    assert(perm.size() == height());

    DLXMatrix res(width());
    for (int i : perm)
        res.add_row(row_to_intvector(rows[i]));
    return res;
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

std::ostream &operator<<(std::ostream &out, const DLXMatrix &M) {
    for (auto &row : M.rows)
        out << M.row_to_boolvector(row) << "\n";
    return out;
}

} // namespace DLX_backtrack
