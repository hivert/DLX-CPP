// Implementation of Knuth dancing links backtrack algorithm
//////////////////////////////////////////////////////////////

// #define DEBUG
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAXNAME 16
#define MAXDEPTH 1000

typedef struct node_s {
    struct node_s *right;
    struct node_s *left;
    struct node_s *up;
    struct node_s *down;
    struct header_s *head;
} node_t, *pNode_t;

typedef struct header_s {
    node_t node;
    char name[MAXNAME];
    int size;
    struct header_s *right;
    struct header_s *left;
} header_t, *pHeader_t;

///////////////////
//  Global vars  //
///////////////////
// Master node of the matrix structure
header_t master;
// Working and stored solution
pNode_t work[MAXDEPTH], solution[MAXDEPTH];
// Depth of the solution
int depthSol = 0;
// Computations statistics
int nSolutions, nChoices, nDances;
// Printing of solution option:
// 0=No, 1=Only one solution, 2=all solutions
int affSol = 1;
// Stop after the first solution
// 0=No, 1=Yes, 2=End of the program
int firstSol = 0;
// Use the random heuristic
int randSol = 0;
// Only prints the number of solutions
int onlyNbSol = 0;

void fatal(char msg[]);
void readmat(void);
pHeader_t findCol(char name[]);
void recordSol(int depth);
void printSol(int depth);

#ifdef DEBUG
void printSize();
void spaceDepth(int depth);
#endif

__attribute__((noreturn)) void fatal(char msg[]) {
    printf("\nFatal error: %s\n", msg);
    exit(EXIT_FAILURE);
}

pHeader_t findCol(char name[]) {
    pHeader_t pHead;
    for (pHead = master.right; pHead != &master; pHead = pHead->right)
        if (strcmp(name, pHead->name) == 0)
            return pHead;
    printf("Unkown column name %s\n", name);
    fatal("Bad matrix");
}

void readmat(void) {
    size_t inputSZ;
    int iRead;
    char *inputLine, *cursor, name[MAXNAME];
    pHeader_t pHead;
    pNode_t pNode;
    node_t lineSeed;

#ifdef DEBUG
    printf("Reading the matrix ");
#endif

    master.name[0] = '\0';
    master.left = master.right = &master;

    inputLine = NULL;
    inputSZ = 0;
    getline(&inputLine, &inputSZ, stdin);
    while (inputLine[0] == '%') {
        if ((inputLine[1] == 'S') && !onlyNbSol)
            printf("%s", inputLine);
        getline(&inputLine, &inputSZ, stdin);
    }

#ifdef DEBUG
    printf("Reading column names\n");
#endif

    for (cursor = inputLine; sscanf(cursor, "%s%n", name, &iRead) > 0;
         cursor += iRead) {
        pHead = malloc(sizeof(*pHead));

#ifdef DEBUG
        printf(".");
#endif
        strncpy(pHead->name, name, MAXNAME);
        pHead->size = 0;

        pHead->right = &master;
        pHead->left = master.left;
        pHead->left->right = master.left = pHead;

        pHead->node.up = pHead->node.down = &pHead->node;
    }
#ifdef DEBUG
    printf("\n");

    for (pHead = master.right; pHead != &master; pHead = pHead->right)
        printf("<%s>", pHead->name);
    printf("\n");

    printf("Creating rows\n");
#endif
    while (getline(&inputLine, &inputSZ, stdin) > 0) {
        // use lineSeed as the seed for building the doubly linked list
        lineSeed.right = lineSeed.left = &lineSeed;

        for (cursor = inputLine; sscanf(cursor, "%s%n", name, &iRead) > 0;
             cursor += iRead) {
            pHead = findCol(name);

            pNode = malloc(sizeof(*pNode));
            pNode->head = pHead;
            pHead->size++;

            pNode->right = &lineSeed;
            pNode->left = lineSeed.left;
            pNode->left->right = lineSeed.left = pNode;

            pNode->down = &pHead->node;
            pNode->up = pHead->node.up;
            pNode->up->down = pHead->node.up = pNode;
        };

#ifdef DEBUG
        for (pNode = lineSeed.right; pNode != &lineSeed; pNode = pNode->right)
            printf("%s ", pNode->head->name);
        printf("\n");
#endif

        // remove the seed lineSeed
        lineSeed.right->left = lineSeed.left;
        lineSeed.left->right = lineSeed.right;
    }

    free(inputLine);
#ifdef DEBUG
    printSize();
#endif
}

#ifdef DEBUG
void printSize() {
    printf("\nsizes: [ ");
    for (pHeader_t pHead = master.right; pHead != &master;
         pHead = pHead->right) {
        int irows;
        pNode_t pNode;
        for (irows = 0, pNode = pHead->node.down; pNode != &pHead->node;
             irows++, pNode = pNode->down) /* Nothing */
            ;
        if (pHead->size != irows)
            fatal("wrong size of column");
        printf("%s:%d ", pHead->name, irows);
    }
    printf("]\n");
}

void spaceDepth(int depth) {
    for (int i = 0; i < depth; i++)
        printf("  ");
}
#endif

void recordSol(int depth) {
    depthSol = depth;
    nSolutions++;
    for (int iDepth = 0; iDepth < depth; iDepth++)
        solution[iDepth] = work[iDepth];
    if (affSol == 2 && !onlyNbSol)
        printSol(depth);
    if (firstSol == 1)
        firstSol = 2;
}

void printRow(pNode_t pRow) {
    printf("%s ", pRow->head->name);
    for (pNode_t pElt = pRow->right; pElt != pRow; pElt = pElt->right)
        printf("%s ", pElt->head->name);
}

void printSol(int depth) {
    printf("Solution:\n");
    for (int iDepth = 0; iDepth < depth; iDepth++) {
        printf(" ");
        printRow(solution[iDepth]);
        printf("\n");
    }
    printf("End\n");
}

void cover(pHeader_t pCol) {
    pCol->left->right = pCol->right;
    pCol->right->left = pCol->left;

    for (pNode_t pRow = pCol->node.down; pRow != &pCol->node;
         pRow = pRow->down) {
        for (pNode_t pElt = pRow->right; pElt != pRow; pElt = pElt->right) {
            pElt->up->down = pElt->down;
            pElt->down->up = pElt->up;
            pElt->head->size--;
            nDances++;
        }
    }
}

void uncover(pHeader_t pCol) {
    pCol->left->right = pCol;
    pCol->right->left = pCol;

    for (pNode_t pRow = pCol->node.up; pRow != &pCol->node; pRow = pRow->up) {
        for (pNode_t pElt = pRow->left; pElt != pRow; pElt = pElt->left) {
            pElt->head->size++;
            pElt->up->down = pElt;
            pElt->down->up = pElt;
        }
    }
}

// Knuth dancing links search algorithm
// Recusive version
///////////////////////////////////////
void search_rec(int depth) {
    pHeader_t pHead, pChooseCol;
    int minSize;

    if (firstSol == 2)
        return;

    if (master.right == &master) {
#ifdef DEBUG
        spaceDepth(depth);
        printf("Got one !!!\n");
#endif
        recordSol(depth);
        return;
    }

    pChooseCol = pHead = master.right;
    minSize = pHead->size;
    for (pHead = pHead->right; pHead != &master; pHead = pHead->right) {
        if (pHead->size < minSize) {
            pChooseCol = pHead;
            minSize = pHead->size;
        }
    }

    if (minSize == 0) {
#ifdef DEBUG
        spaceDepth(depth);
        printf("Too Bad !!!\n");
#endif
        return;
    }

    if (minSize > 1) {
        if (randSol == 1) {
            int nCol = -1;
            for (pHead = master.right; pHead != &master;
                 pHead = pHead->right, nCol++)
                /* Nothing */;
            int randCol = 1 + (int)(1.0 * nCol * rand() / (RAND_MAX + 1.0));
            pChooseCol = master.right;
            for (int iCol = 0; iCol < randCol; iCol++)
                pChooseCol = pChooseCol->right;
        } else if (randSol == 2) {
            // Choose a column at random amoung the minimal ones
            int nCol = -1;
            for (pHead = master.right; pHead != &master; pHead = pHead->right)
                if (pHead->size == minSize) nCol++;

                /* Nothing */;
            int randCol = 1 + (int)(1.0 * nCol * rand() / (RAND_MAX + 1.0));
            pChooseCol = master.right;
            for (int iCol = 0; iCol < randCol; pChooseCol = pChooseCol->right)
                if (pChooseCol->size == minSize) iCol++;
        }
    }

    cover(pChooseCol);
    for (pNode_t pRow = pChooseCol->node.down; pRow != &pChooseCol->node;
         pRow = pRow->down) {
        nChoices++;
        work[depth] = pRow;
        for (pNode_t pElt = pRow->right; pElt != pRow; pElt = pElt->right)
            cover(pElt->head);
        search_rec(depth + 1);
        for (pNode_t pElt = pRow->left; pElt != pRow; pElt = pElt->left)
            uncover(pElt->head);
    }
    uncover(pChooseCol);
}

int main(int argc, char *const argv[]) {
    struct timespec before, after;
    long long nparse, nsec, nprint;
    char opt;

    while ((opt = getopt(argc, argv, "nrRif012")) != EOF)
        switch (opt) {
        case '0':
        case '1':
        case '2':
            affSol = opt - '0';
            break;
        case 'f':
            firstSol = 1;
            break;
        case 'r':
            randSol = 1;
            break;
        case 'R':
            randSol = 2;
            break;
        case 'n':
            onlyNbSol = 1;
            break;
        default:
            fatal("Unkown option");
        }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &before);
    readmat();
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &after);
    nparse = ((long long)1000000000) * (after.tv_sec - before.tv_sec) +
             (after.tv_nsec - before.tv_nsec);
    if (randSol) {
        srand(after.tv_nsec);
    }

    nSolutions = 0;
    nChoices = 0;
    nDances = 0;

    if (!onlyNbSol)
        printf("%%C Go for it\n");
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &before);

    search_rec(0);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &after);
    nsec = ((long long)1000000000) * (after.tv_sec - before.tv_sec) +
           (after.tv_nsec - before.tv_nsec);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &before);
    if (affSol == 1)
        printSol(depthSol);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &after);
    nprint = ((long long)1000000000) * (after.tv_sec - before.tv_sec) +
             (after.tv_nsec - before.tv_nsec);
    if (!onlyNbSol) {
        printf("%%T Number of solutions: %d\n", nSolutions);
        printf("%%T Number of choices: %d, Number of dances: %d\n", nChoices,
               nDances);
        printf("%%T Timings: parse = %lld ns, solve = %lld ns, output = %lld "
               "ns, total = %lld ns\n",
               nparse, nsec, nprint, nparse + nsec + nprint);
    } else {
        printf("%d\n", nSolutions);
    }

    return EXIT_SUCCESS;
}

// clang-format off
/*

Pour savoir s'il y a plusieurs solutions
cat examples/sudoku3.txt | ./sudoku2dance | ./fabrik -n -0
1
cat examples/template4x3.txt | ./sudoku2dance | ./fabrik -n -0
2

Pour tirer une solution aléatoire
cat examples/template4x3.txt | ./sudoku2dance | ./fabrik -f -r | ./sol2mupad
//Standard sudoku (block size = 4x3, square size = 12)
[[  4, 2, 1, 8, 5, 6, 9, 10, 7, 12, 3, 11, 0],
 [  7, 3, 6, 5, 8, 11, 2, 12, 4, 1, 9, 10, 0],
 [  10, 9, 12, 11, 1, 7, 3, 4, 5, 6, 2, 8, 0],
 [  3, 1, 11, 4, 2, 8, 12, 6, 9, 5, 10, 7, 0],
 [  9, 12, 5, 2, 7, 10, 1, 11, 6, 4, 8, 3, 0],
 [  8, 6, 7, 10, 9, 4, 5, 3, 11, 2, 12, 1, 0],
 [  5, 8, 9, 12, 6, 2, 10, 7, 3, 11, 1, 4, 0],
 [  6, 11, 10, 1, 3, 9, 4, 8, 2, 7, 5, 12, 0],
 [  2, 7, 4, 3, 12, 5, 11, 1, 10, 8, 6, 9, 0],
 [  1, 10, 3, 7, 4, 12, 6, 2, 8, 9, 11, 5, 0],
 [  12, 5, 8, 6, 11, 3, 7, 9, 1, 10, 4, 2, 0],
 [  11, 4, 2, 9, 10, 1, 8, 5, 12, 3, 7, 6, 0],0]
*/
// clang-format on
