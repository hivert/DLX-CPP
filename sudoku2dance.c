#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printCaseOcc(int row, int col, int number,
		  int sqSize, int block[sqSize][sqSize])
{
    printf("r_%d_%d ", row, number);
    printf("c_%d_%d ", col, number);
    printf("s_%d_%d ", row, col);
    printf("b_%d_%d ", block[row-1][col-1], number);
}


int main(void)
{
    int i, j, n;
    int nHint, rowSize, colSize, sqSize;
    char type, cc;

    // Scanning the size
    scanf("%c", &type);
    if (type=='s')	
    {
	scanf("%dx%d", &colSize, &rowSize);
	sqSize = colSize*rowSize;
	printf("%%S s%dx%d\n", colSize, rowSize);
	printf("%%C Standard sudoku (block size = %dx%d, square size = %d)\n",
	       colSize, rowSize, sqSize);
    }
    else if (type=='g')
    {
	scanf("%d", &sqSize);
	printf("%%S g%d\n", sqSize);
	printf("%%C Generalized sudoku %dx%d\n", sqSize, sqSize);
    } else {
	fprintf(stderr, "Unknown block type <%c>\n", type);
	exit(EXIT_FAILURE);
    }
    
    { // Dynamic allocation of the matrices. 
	int block[sqSize][sqSize];
	int matrix[sqSize][sqSize];    
    
	// List of the columns of the matrix
	// Square i,j occupied
	for (i = 1; i <= sqSize; i++)
	    for (j = 1; j <= sqSize; j++)
		printf("s_%d_%d ", j, i);

	// Block i occupied by j
	for (i = 1; i <= sqSize; i++)
	    for (j = 1; j <= sqSize; j++)
		printf("b_%d_%d ", j, i);

	// Row i occupied by j
	for (i = 1; i <= sqSize; i++)
	    for (j = 1; j <= sqSize; j++)
		printf("r_%d_%d ", j, i);

	// Col i occupied by j
	for (i = 1; i <= sqSize; i++)
	    for (j = 1; j <= sqSize; j++)
		printf("c_%d_%d ", j, i);

	if (type=='s') // Standard block structure
	    for (i = 0; i < sqSize; i++)
		for (j = 0; j < sqSize; j++)
		    block[i][j] = j / colSize + rowSize*(i / rowSize)+1;
	else // Generalized standard block structure
	    for (i = 0; i < sqSize; i++)
		for (j = 0; j < sqSize; j++)
		    scanf("%d", &block[i][j]);
	
	// Hint of the enonce of the problem
	nHint = 0;
	for (i = 0; i < sqSize; i++)
	    for (j = 0; j < sqSize; j++)
	    {
		if (scanf("%d", &matrix[i][j]) == 0)
		{
		    if 	((cc = getchar()) == '.')
			matrix[i][j] = 0;
		    else	
		    {	
			fprintf(stderr, "Bad character <%c>\n", cc);
			exit(EXIT_FAILURE);
		    }
		}
		if (matrix[i][j] != 0) nHint++;
	    }
    
	for (i = 0; i < nHint; i++)
	    printf("en_%03d ", i);
    
	printf("\n");
	// End of list of the columns of the matrix

    
	// Rule of the Sudoku game
	for (i = 1; i <= sqSize; i++)
	    for (j = 1; j <= sqSize; j++)
		for (n = 1; n <= sqSize; n++)
		{
		    printCaseOcc(i, j, n, sqSize, block);
		    printf("\n");
		}

	// Enonce of the particular problem
	nHint =0;
	for (i = 1; i <= sqSize; i++)
	    for (j = 1; j <= sqSize; j++)
		if (matrix[i-1][j-1] != 0)
		{
		    printCaseOcc(i, j, matrix[i-1][j-1], sqSize, block);
		    printf(" en_%03d\n", nHint);
		    nHint++;
		}
    }
    scanf("%d", &i);
    return EXIT_SUCCESS;
}
