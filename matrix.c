 #include <stdio.h>
 #include "matrix.h"


void printmatrix(matrix *mymatrix)
{
    int row, column;
    for(row = 0; row < (2 * mymatrix->length + 1); row++){
        /* colonna */
        for (column = 0; column < (2 * mymatrix->heigth + 1); column++) {
            if(row % 2 == 0){
                printf("-");
            }
            else if(row % 2 != 0 && column % 2 == 0){
                printf("|");
            }
            else if(row % 2 != 0 && column % 2 != 0){
                printf("%c", mymatrix->table[row][column]);
            }
        }
        printf("\n");
    }
}

int insert(matrix *mymatrix, char what, int where)
{
    if(where >= mymatrix->length)
    {
        printf("Errore, la seguente colonna non esiste, riprova");
        return(-1);
    }

    else if (mymatrix->table[0][where] != ' ')
    {
        printf("Errore, la seguente colonna è già piena, riprova");
        return(-1);
    }
    
    int row = 0;
    while(row < mymatrix->heigth)
    {
        if(mymatrix->table[row + 1][where] != ' '){
            mymatrix->table[row][where] == what;
            return(0);
        }
        row++;
    }
    mymatrix->table[row][where] == what;
    return(0);
}

bool checkwin(matrix *mymatrix, bool checkdiagonal)
{
    
}


