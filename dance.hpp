// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////
#include <utility>
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <stack>


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

    void print_columns() const;
    void check_sizes() const;

    int add_row(const std::vector<int>);

    void search(int);
    void print_solution() const;

protected:

    DLXMatrix() = delete;
    DLXMatrix( const DLXMatrix& ) = delete; // non construction-copyable
    DLXMatrix& operator=( const DLXMatrix& ) = delete; // non copyable

private:

    Header *master() { return &heads[0]; }
    const Header *master() const { return &heads[0]; }

    Header *choose_min();
    void cover(Header *);
    void uncover(Header *);

    void search_rec(int);

    std::vector<Header> heads;
    std::vector<std::vector<Node>> rows;

    std::vector<Node *> work, solution;
    int nb_solutions, nb_choices, nb_dances;
};
