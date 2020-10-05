// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////
#include <utility>
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <stack>

struct Header;
struct Node {
    int row_id;
    Node *left, *right, *up, *down;
    Header *head;
};

struct Header {
    Node node;
    int col_id, size;
    Header *left, *right;
};


class DLXMatrix {
public:
    DLXMatrix(int nb_col);

    void print_columns() const;
    void check_sizes() const;

    Header *master() { return &heads[0]; }
    const Header *master() const { return &heads[0]; }

    void new_row(int, const std::vector<int>);

    void cover(Header *);
    void uncover(Header *);

    void search_rec(int);
    void search(int);
    void print_solution() const;

private:

    Header *choose_min();

    std::vector<Header> heads;
    std::vector<std::vector<Node>> rows;

    std::vector<Node *> work, solution;
    int nb_solutions, nb_choices, nb_dances;
};
