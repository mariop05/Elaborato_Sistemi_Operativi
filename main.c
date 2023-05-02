#include <stdio.h>
#include "matrix.h"
#define COLUMN 5
#define ROW 5
int main(int argc, char *argv[]) {
    
    matrix mymatrix;

    mymatrix.heigth = ROW;
    mymatrix.length = COLUMN;

    for(int row = 0; row < ROW; row ++){
        for (int column = 0; column < COLUMN; column++){
            mymatrix.table[row][column] = ' ';
        }
    }

    int turn = 0;
    char symbol;
    int insertcol;
    while (1)
    {
        if(turn % 2 == 0)
            symbol = 'X';
        else
            symbol = 'O';

        printf("Inserisci la mossa %c: ", symbol);

        do{
            scanf("%d", &insertcol);
        }
        while(insert(&mymatrix, symbol, insertcol) != 0);
        

        printmatrix(&mymatrix);

        if(checkwin(&mymatrix, 1)){
            printf("Complimenti %c hai vinto!!\n", symbol);
            return 0;
        }

        turn++;
    }
    
    

    printmatrix(&mymatrix);
}
