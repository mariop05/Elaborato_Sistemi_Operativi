 #include <stdio.h>
 #include "matrix.h"


 void printmatrix(matrix *mymatrix)
 {
     int row, column;
     /* riga */
     for(row = 0; row < (2 * mymatrix->heigth + 1); row++){
         /* colonna */
         for (column = 0; column < (2 * mymatrix->length + 1); column++) {
             if(row % 2 == 0){
                 printf("-");
             }
             else if(row % 2 != 0 && column % 2 == 0){
                 printf("|");
             }
             else if(row % 2 != 0 && column % 2 != 0){
                 printf("%c", mymatrix->table[row/2][column/2]);
             }
         }
         printf("\n");
     }
 }

 int insert(matrix *mymatrix, char what, int where)
 {
     if(where >= mymatrix->length)
     {
         printf("Errore, la seguente colonna non esiste, riprova\n");
         return(-1);
     }

     else if (mymatrix->table[0][where] != ' ')
     {
         printf("Errore, la seguente colonna è già piena, riprova\n");
         return(-1);
     }

     int row = 0;
     while(row < mymatrix->heigth - 1)
     {
         if(mymatrix->table[row + 1][where] != ' '){
             mymatrix->table[row][where] = what;
             return(0);
         }
         row++;
     }
     mymatrix->table[row][where] = what;
     return(0);
 }

 int checkwin(matrix *mymatrix, int checkdiagonal)
 {
     int row, column;
     char symbol;
     int parity = 0;

     for(row = 0; row < mymatrix->heigth; row++){
         for (column = 0; column < mymatrix->length - 3; column++) {
             symbol = mymatrix->table[row][column];

             if (symbol != ' ' &&
                 symbol == mymatrix->table[row][column + 1] &&
                 symbol == mymatrix->table[row][column + 2] &&
                 symbol == mymatrix->table[row][column + 3]
                     )
             {
                 return (1);
             }
         }
     }

     for(column = 0; column < mymatrix->length; column++){
         for (row = 0; row < mymatrix->heigth - 3; row++) {
             symbol = mymatrix->table[row][column];

             if (symbol != ' ' &&
                 symbol == mymatrix->table[row + 1][column] &&
                 symbol == mymatrix->table[row + 2][column] &&
                 symbol == mymatrix->table[row + 3][column]
                     )
             {
                 return (1);
             }
         }
     }

     if(checkdiagonal){
         for(row = 0; row < mymatrix->heigth - 3; row++){
             for (column = 0; column < mymatrix->length - 3; column++) {
                 symbol = mymatrix->table[row][column];

                 if (symbol != ' ' &&
                     symbol == mymatrix->table[row + 1][column + 1] &&
                     symbol == mymatrix->table[row + 2][column + 2] &&
                     symbol == mymatrix->table[row + 3][column + 3]
                         )
                 {
                     return (1);
                 }
             }
         }

         for(row = 0; row < mymatrix->heigth - 3; row++){
             for (column = mymatrix->length - 1; column >= 3; column--) {
                 symbol = mymatrix->table[row][column];

                 if (symbol != ' ' &&
                     symbol == mymatrix->table[row + 1][column - 1] &&
                     symbol == mymatrix->table[row + 2][column - 2] &&
                     symbol == mymatrix->table[row + 3][column - 3]
                         )
                 {
                     return (1);
                 }
             }
         }
     }

     for (column = 0; column < mymatrix->length; column++)
         if (mymatrix->table[0][column] != ' ')
             parity++;

     if (parity == mymatrix->length)
     {
         return(2);
     }

     return(0);
 }
 void initializematrix(matrix *mymatrix)
 {
     for(int row = 0; row < mymatrix->heigth; row ++)
         for(int column = 0; column < mymatrix->length; column ++)
             mymatrix->table[row][column] = ' ';
 }

