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

    int add_row(const std::vector<int>&);

    std::vector<std::vector<int>> search_rec(int);
    bool search_iter();
    bool search_iter(std::vector<int> &);
    void reset();

    std::vector<int> get_solution();

    DLXMatrix(const DLXMatrix &);
    DLXMatrix& operator=(DLXMatrix other);

    int nb_solutions, nb_choices, nb_dances;

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
    void print_solution(const std::vector<Node *> &) const;

    std::vector<Header> heads;
    std::vector<std::vector<Node>> rows;

    std::vector<Node *> work;
    bool search_down;

};
