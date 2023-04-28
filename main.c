#include <stdio.h>
#define COLUMN 5
#define ROW 5
int main(int argc, char *argv[]) {
    int row, column;
    char matrice[2*ROW + 1][2*COLUMN + 1];
    char carattere = *argv[1];
    if(argc != 2)
        return -1;

    printf("carattere: %c\n", carattere);
    if(carattere != 'X')
        return -1;


    /* riga */
    for(row = 0; row < (2 * ROW + 1); row++){
        /* colonna */
        for (column = 0; column < (2 * COLUMN + 1); column++) {
            if(row % 2 == 0){
                matrice[row][column] = '-';
            }
            else if(row % 2 != 0 && column % 2 == 0){
                matrice[row][column] = '|';
            }
            else if(row % 2 != 0 && column % 2 != 0){
                matrice[row][column] = ' ';
            }
        }
    }

    matrice[1][1] = 'X';

    /* stampo la matrice */
    for(row = 0; row < (2 * ROW + 1); row++){
        for (column = 0; column < (2 * COLUMN + 1); column++) {
            printf("%c", matrice[row][column]);
        }
        printf("\n");

    }
}
