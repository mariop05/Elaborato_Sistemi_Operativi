#pragma once
#ifndef _MATRIX_HH
#define _MATRIX_HH

#include <stdbool.h> 

typedef struct
{
    int heigth;
    int length;

    char table[10][10];

}matrix;

//print the matrix
void printmatrix(matrix *mymatrix);

//insert in a specified row(where) a specified character(what)
//returns -1 if it fails
int insert(matrix *mymatrix, char what, int where);

//check the win
int checkwin(matrix *mymatrix, int checkdiagonal/*0 if not diadonal, 1 if yes*/);




#endif