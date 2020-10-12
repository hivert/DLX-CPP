// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////
#include <utility>
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <stack>
#include <iostream>

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

    DLXMatrix(int nb_col);
    DLXMatrix(const DLXMatrix &);
    DLXMatrix& operator=(DLXMatrix other);

    size_t width() const { return heads.size() - 1; }
    size_t height() const { return rows.size(); }

    void print_columns() const;
    void check_sizes() const;

    int add_row(const std::vector<int>&);

    std::vector<std::vector<int>> search_rec(int);
    bool search_iter();
    bool search_iter(std::vector<int> &);
    std::vector<int> get_solution();

    void reset();

    int nb_solutions, nb_choices, nb_dances;

    DLXMatrix permuted_columns(const std::vector<int> &);
    DLXMatrix permuted_rows(const std::vector<int> &);

    friend std::ostream & operator<< (std::ostream &, const DLXMatrix &);

protected:

    DLXMatrix() = delete;

    Header *master() { return &heads[0]; }
    const Header *master() const { return &heads[0]; }

    Header *choose_min();
    void cover(Header *);
    void uncover(Header *);
    void choose(Node *);
    void unchoose(Node *);
    void search_rec_internal(int, std::vector<std::vector<int>> &);

    static std::vector<int> row_to_intvector(const std::vector<Node>&);
    std::vector<bool> row_to_boolvector_slow(const std::vector<Node>&) const;
    std::vector<bool> row_to_boolvector(const std::vector<Node>&) const;
    void print_solution(const std::vector<Node *> &) const;

    std::vector<Header> heads;
    std::vector<std::vector<Node>> rows;

    std::vector<Node *> work;
    bool search_down;

};

template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
    out << '[';
    for (auto i : v) std::cout << i << ", ";
    out << "\b\b]";
    return out;
}
