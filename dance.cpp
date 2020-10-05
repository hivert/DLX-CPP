// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////
#include <cstring>
#include <iostream>
#include "dance.hpp"


__attribute__((noreturn)) void fatal(const char *msg) {
    std::cerr << "\nFatal error: " << msg << std::endl;
    exit(EXIT_FAILURE);
}

void DLXMatrix::new_column(const char * name) {
    Header *pHead = new Header();

    std::strncpy(pHead->name, name, MAXNAME);
    pHead->size = 0;

    pHead->right = &master;
    pHead->left = master.left;
    pHead->left->right = master.left = pHead;

    pHead->node.up = pHead->node.down = &pHead->node;
}
void DLXMatrix::new_column(const std::string name) {
    new_column(name.c_str());
}

void DLXMatrix::print_columns() const {
    for (Header *h = master.right; h != &master; h = h->right) {
        std::cout << h->name << "(" << h->size << ") ";
    }
    std::cout << std::endl;
}

Header *DLXMatrix::find_column(const char name[]) {
    Header *pHead;
    for (pHead = master.right; pHead != &master; pHead = pHead->right)
        if (strcmp(name, pHead->name) == 0)
            return pHead;
    std::cerr << "Unkown column name \"" << name << "\"" << std::endl;
    fatal("Bad matrix");
}
Header *DLXMatrix::find_column(const std::string s) {
    return find_column(s.c_str());
}

int main() {
    DLXMatrix M;

    M.new_column("toto");
    M.new_column("tata");
    M.new_column("tu");
    M.new_column("titi");
    M.print_columns();

    std::cout << M.find_column("tata") << std::endl;

    std::string s = "tat";
    std::cout << M.find_column(s) << std::endl;
}
