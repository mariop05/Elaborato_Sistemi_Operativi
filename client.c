#include <stdio.h>
#include "err_exit.h"

char *username;

int main(int argc, char const *argv[])
{
    if (argc != 2)
        ErrExit("Sintassi sbagliata\nclient nomeutente");

    *username = argv[1];
    
    return 0;
}
