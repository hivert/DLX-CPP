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
#ifndef DANCE_HPP_
#define DANCE_HPP_

#include <iostream>
#include <vector>

std::vector<int> inverse_perm(const std::vector<int> &perm);

class DLXMatrix {
    struct Header;
    struct Node {
        int row_id;
        Node *left, *right, *up, *down;
        Header *head;
    };

    struct Header {
        int col_id, size;
        Node node;
        Header *left, *right;
    };

  public:
    explicit DLXMatrix(int nb_col);
    DLXMatrix(const DLXMatrix &);
    DLXMatrix &operator=(DLXMatrix other);

    size_t width() const { return heads.size() - 1; }
    size_t height() const { return rows.size(); }

    void print_columns() const;
    void check_sizes() const;

    int add_row(const std::vector<int> &row);
    std::vector<int> int_row(size_t i) const ;
    std::vector<bool> bool_row(size_t i) const;

    std::vector<std::vector<int>> search_rec(int max_sol);
    bool search_iter();
    bool search_iter(std::vector<int> &);
    std::vector<int> get_solution();
    bool search_random(std::vector<int> &);

    void reset();

    int nb_solutions, nb_choices, nb_dances;

    DLXMatrix permuted_columns(const std::vector<int> &perm);
    DLXMatrix permuted_inv_columns(const std::vector<int> &inv);
    DLXMatrix permuted_rows(const std::vector<int> &perm);

    friend std::ostream &operator<<(std::ostream &, const DLXMatrix &);

  protected:
    DLXMatrix() = delete;

    Header *master() { return &heads[0]; }
    const Header *master() const { return &heads[0]; }

    Header *choose_min();
    void cover(Header *h);
    void uncover(Header *h);
    void choose(Node *n);
    void unchoose(Node *n);
    void search_rec_internal(int, std::vector<std::vector<int>> &);

    static std::vector<int> row_to_intvector(const std::vector<Node> &);
    std::vector<bool> row_to_boolvector_slow(const std::vector<Node> &) const;
    std::vector<bool> row_to_boolvector(const std::vector<Node> &) const;
    void print_solution(const std::vector<Node *> &) const;

    std::vector<Header> heads;
    std::vector<std::vector<Node> > rows;

    std::vector<Node *> work;
    bool search_down;
};

template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v) {
    out << '[';
    for (auto i : v)
        std::cout << i << ", ";
    out << "\b\b]";
    return out;
}

#endif  // DANCE_HPP_
