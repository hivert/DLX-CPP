// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////
#include <cstring>
#include <iostream>
#include "dance.hpp"


__attribute__((noreturn)) void fatal(const char *msg) {
    std::cerr << "\nFatal error: " << msg << std::endl;
    exit(EXIT_FAILURE);
}


DLXMatrix::DLXMatrix(int nb_col) : heads(nb_col+1) {
    for (int i = 0; i <= nb_col; i++) {
        heads[i].size  = 0;
        heads[i].col_id = i-1;
        heads[i].node.row_id = -1;
        heads[i].node.head = &heads[i];
        heads[i].node.left = heads[i].node.right = NULL; // unused
        heads[i].node.up = heads[i].node.down = &heads[i].node;
    }
    heads[nb_col].right = &heads[0];
    for (int i = 0; i < nb_col; i++) heads[i].right = &heads[i+1];
    heads[0].left = &heads[nb_col];
    for (int i = 1; i <= nb_col; i++) heads[i].left = &heads[i-1];
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
        for (Node *p = h->node.down; p != &h->node; irows++, p = p->down)
            /* Nothing */;
        if (h->size != irows)
            fatal("wrong size of column");
        std::cout << h->col_id << "(" << irows << ") ";
    }
    printf("]\n");
}

int DLXMatrix::add_row(const std::vector<int> r) {
    int row_id = rows.size();
    rows.push_back(std::vector<Node>(r.size()));
    std::vector<Node> & row = rows[rows.size()-1];

    for (size_t i = 0; i < r.size(); i++) {
        unsigned int icol = r[i]+1;
        if (not (0 < icol and icol < heads.size()))
            fatal("No such column !");
        row[i].row_id = row_id;
        row[i].head = &heads[icol];
        heads[icol].size++;
        row[i].down = &heads[icol].node;
        row[i].up = heads[icol].node.up;
        row[i].up->down = heads[icol].node.up = &row[i];
    }
    row[r.size()-1].right = &row[0];
    for (size_t i = 0; i < r.size()-1; i++) row[i].right = &row[i+1];
    row[0].left = &row[r.size()-1];
    for (size_t i = 1; i < r.size(); i++) row[i].left = &row[i-1];
    return row_id;
}


void DLXMatrix::cover(Header *pCol) {
    pCol->left->right = pCol->right;
    pCol->right->left = pCol->left;

    for (Node *pRow = pCol->node.down; pRow != &pCol->node;
         pRow = pRow->down) {
        for (Node *pElt = pRow->right; pElt != pRow; pElt = pElt->right) {
            pElt->up->down = pElt->down;
            pElt->down->up = pElt->up;
            pElt->head->size--;
            nb_dances++;
        }
    }
}

void DLXMatrix::uncover(Header *pCol) {
    pCol->left->right = pCol;
    pCol->right->left = pCol;

    for (Node *pRow = pCol->node.up; pRow != &pCol->node; pRow = pRow->up) {
        for (Node *pElt = pRow->left; pElt != pRow; pElt = pElt->left) {
            pElt->head->size++;
            pElt->up->down = pElt;
            pElt->down->up = pElt;
        }
    }
}


void DLXMatrix::print_solution() const {
    std::cout << "Solution:\n";
    for (Node * n : solution) {
        std::cout << " " << n->row_id << " :";
        for (const Node &i : rows[n->row_id])
            std::cout << " " << i.head->col_id;
        std::cout << "\n";
    }
    std::cout << "End" << std::endl;
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

void DLXMatrix::search(int maxsol) {
    nb_solutions = nb_choices = nb_dances = 0;
    search_rec(maxsol);
}

// Knuth dancing links search algorithm
// Recusive version
///////////////////////////////////////
void DLXMatrix::search_rec(int maxsol) {
    if (master()->right == master()) {
        nb_solutions++;
        solution = work;
        print_solution();
        return;
    }

    Header *choice = choose_min();
    if (choice->size == 0) {
        return;
    }

    cover(choice);
    for (Node *r = choice->node.down; r != &choice->node; r = r->down) {
        nb_choices++;
        work.push_back(r);
        for (Node *nr = r->right; nr != r; nr = nr->right) cover(nr->head);
        search_rec(maxsol);
        for (Node *nr = r->left; nr != r; nr = nr->left) uncover(nr->head);
        work.pop_back();
        if (nb_solutions >= maxsol) break;
    }
    uncover(choice);
}


int main() {
    DLXMatrix M(5);
    M.add_row({0,2});
    M.add_row({0,1});
    M.add_row({1,4});
    M.add_row({3});
    M.add_row({3,4});
    M.add_row({2,3,4});
    M.print_columns();
    M.check_sizes();
    M.search(2);
}
