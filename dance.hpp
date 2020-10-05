// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////
#include <utility>
#include <cstdint>
#include <array>
#include <string>

const std::size_t MAXNAME = 16;
const int MAXDEPTH = 1000;

struct Header;
struct Node {
    Node *left, *right, *up, *down;
    Header *head;
};

struct Header {
    Node node;
    char name[MAXNAME];
    int size;
    Header *left, *right;
};


class DLXMatrix {
public:
    DLXMatrix() {
        master.name[0] = '\0';
        master.left = master.right = &master;
    }

    void new_column(const char *);
    void new_column(const std::string);

    void print_columns() const;
    Header *find_column(const char name[]);
    Header *find_column(const std::string);

private:
    Header master;
    std::array<Node *, MAXDEPTH> work, solution;
    int depthSol = 0;
    int nSolutions = 0, nChoices = 0, nDances = 0;
};
