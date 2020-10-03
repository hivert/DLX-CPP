#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXNAME 16

void fatal(char  msg[])
{
    printf("\nFatal error: %s\n", msg);
    exit(EXIT_FAILURE);    
}

void set(int *dst, int src)
{
    if ((*dst != 0) && (*dst != src))
	fatal("Imcompatible solutions");
    *dst = src;
}


int main(void)
{
    int sqSize, colSize=0, rowSize=0; 
    int i, j, row, col, n;
    char *inputLine, *cursor;
    unsigned int inputSZ, iRead;
    char name[MAXNAME];
    char format[]="%2d ";

    inputLine = NULL;
    inputSZ   = 0;
    do {
	getline(&inputLine, &inputSZ, stdin);
	if ((inputLine[0]=='%') && (inputLine[1]=='S'))
	{	
	    if (inputLine[3]=='s')	
	    {
		sscanf(inputLine+4, "%dx%d", &colSize, &rowSize);
		sqSize = colSize*rowSize;
		printf("Standard sudoku (block size = %dx%d, square size = %d)\n",
		       colSize, rowSize, sqSize);
	    }
	    else if (inputLine[3]=='g')
	    {
		sscanf(inputLine+4, "%d", &sqSize);
		printf("Generalized sudoku %dx%d\n\n", sqSize, sqSize);
	    }
	}	
    } while (strcmp(inputLine, "Solution:\n") != 0);

    // Computing the printing format
    for (i=sqSize, j=0;  i != 0;  j++, i/=10) {};
    format[1] = j+'0';
    
    {
	int matrix[sqSize][sqSize];

	for (i = 0; i < sqSize; i++)
	    for (j = 0; j < sqSize; j++)
		matrix[i][j] = 0;    

	getline(&inputLine, &inputSZ, stdin);
	do {
	    row = col = n = 0;
	    for(cursor = inputLine;
		sscanf(cursor, " %[^_]_%d_%d%n", name, &i, &j, &iRead) > 0;
		cursor +=iRead)
	    {
		switch (name[0])
		{
		    case 'e' :
			sscanf(cursor, " %s%n", name, &iRead);
			cursor +=iRead;	
			break;
		    case 'b' :
			break;
		    case 'r' :
			set(&row, i); set(&n, j); break;
		    case 'c' :
			set(&col, i); set(&n, j); break;
		    case 's' :
			set(&row, i); set(&col, j); break;
		    default :
			printf("Token: <%s_%d_%d>\n", name, i, j);
			fatal("Unkown solution token");
		}
	    }

	    set(&(matrix[row-1][col-1]), n);
	
	    getline(&inputLine, &inputSZ, stdin);
	} while (strcmp(inputLine, "End\n") != 0);

	
	for (i = 0; i < sqSize; i++)
	{
	    if ((rowSize != 0) && !(i % rowSize)) printf("\n");
	    printf("  ");
	    for (j = 0; j < sqSize; j++)
	    {
		if ((colSize != 0) && !(j % colSize)) printf(" ");
		printf(format	, matrix[i][j]);
	    }
	
	    printf("\n");
	}
	printf("\n");
	while (getline(&inputLine, &inputSZ, stdin) > 0)
	    if (inputLine[0]=='%' && inputLine[1]=='T')
		printf("%s", inputLine+3);	
    }    
    return EXIT_SUCCESS;
}

    

