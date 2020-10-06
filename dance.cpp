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
    std::vector<Node> & row = rows.back();

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
    row.back().right = &row[0];
    for (size_t i = 0; i < r.size()-1; i++) row[i].right = &row[i+1];
    row[0].left = &row.back();
    for (size_t i = 1; i < r.size(); i++) row[i].left = &row[i-1];
    return row_id;
}


void DLXMatrix::print_solution(const std::vector<Node *> &solution) const {
    std::cout << "Solution: (size = " << work.size() << ") : \n";
    for (const Node *n : solution) {
        std::cout << " " << n->row_id << " :";
        for (const Node &i : rows[n->row_id])
            std::cout << " " << i.head->col_id;
        std::cout << "\n";
    }
    std::cout << "End" << std::endl;
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
    for (Node *nr = row->right; nr != row; nr = nr->right) cover(nr->head);
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
    for (Node *nr = row->left; nr != row; nr = nr->left) uncover(nr->head);
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
void DLXMatrix::search(int maxsol) {
    nb_solutions = nb_choices = nb_dances = 0;
    search_rec(maxsol);
}
void DLXMatrix::search_rec(int maxsol) {
    if (master()->right == master()) {
        nb_solutions++;
        print_solution(work);
        return;
    }

    Header *choice = choose_min();
    if (choice->size == 0) return;

    cover(choice);
    for (Node *row = choice->node.down; row != &choice->node; row = row->down) {
        choose(row);
        search_rec(maxsol);
        unchoose(row);
        if (nb_solutions >= maxsol) break;
    }
    uncover(choice);
}


// Knuth dancing links search algorithm
// Iterative version
///////////////////////////////////////
void DLXMatrix::search_iter(int maxsol) {
    nb_solutions = nb_choices = nb_dances = 0;
    do {
	while (true) { // going down the recursion
	    if (master()->right == master()) {
                nb_solutions++;
                print_solution(work);
                if (nb_solutions >= maxsol) {
                    while (not work.empty()) {
                        Node *row = work.back();
                        unchoose(row);
                        uncover(row->head);
                    }
                    return;
                }
		break;
	    }
            Header *choice = choose_min();
            if (choice->size == 0) {
                work.pop_back();
                break;
            }
            cover(choice);
            choose(choice->node.down);
        }
	while (not work.empty()) { // going up the recursion
            Node *row = work.back();
            Header *choice = row->head;
            unchoose(row);
	    row = row->down;
	    if (row != &choice->node) {
                choose(row);
                break;
	    }
            uncover(choice);
	}
    } while (not work.empty());
}


int main() {
    DLXMatrix M(6);

    M.add_row({0,2});
    M.add_row({0,1});
    M.add_row({1,4});
    M.add_row({3});
    M.add_row({3,4});
    M.add_row({5});
    M.add_row({1});
    M.add_row({0,1,2});
    M.add_row({2,3,4});
    M.add_row({1,4,5});
    M.check_sizes();
    const int maxsol = 3;
    std::cout << "Recursive ========================= \n";
    M.search(maxsol);
    std::cout << "Iterative ========================= \n";
    M.search_iter(maxsol);
}
 
