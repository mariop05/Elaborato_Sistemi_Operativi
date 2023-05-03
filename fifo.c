/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "err_exit.h"
#include "fifo.h"

int createFifo(char *pathname, mode_t mode){
    int fifo = mkfifo(pathname, mode);
    if(fifo == -1)
        ErrExit("error make fifo");
    return fifo;
}
int openFifo(char *pathname, int flags){
    int fd = open(pathname, flags);
    if(fd == -1)
        ErrExit("error open fifo");
    return fd;
}
void closeFifo(int fd){
    if(close(fd) == -1)
        ErrExit("error fifo");
}
void removeFifo(char *pathname){
    if(unlink(pathname) == -1)
        ErrExit("error unlink");
}
