#include <stdio.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <signal.h>
#include "err_exit.h"
#include "matrix.h"
#include "semaphore.h"
#include "shared_memory.h"


#define SEM_KEY 10
#define SHM_KEY 11

char *username;
matrix mymatrix;

void siginthandler(int sig){
    printf("The signal ctrl-c was caught!\n");
}

void sigusrhandler(int sig){
    
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
        ErrExit("Sintassi sbagliata\nclient nomeutente");

    char *username = argv[1];
    
    int semid = semget(SEM_KEY, 3 , S_IRUSR | S_IWUSR);

    if (semid == -1)
        errExit("Errore nella configurazione dei semafori (forse il server non è in esecuzione?)");

    int shmid = alloc_shared_memory(SHM_KEY, sizeof(mymatrix));
    
    return 0;
}
