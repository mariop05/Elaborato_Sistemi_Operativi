#include <stdio.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "err_exit.h"
#include "matrix.h"
#include "semaphore.h"
#include <unistd.h>
#include "shared_memory.h"
#include "utils.h"


#define SEM_KEY 10
#define SHM_KEY 11

const char *fifoclient2serverpath = "./client2server";
const char *fifosever2clientpath = "./server2client";

char *username; //the client username
matrix mymatrix; //the client matrix
pid_t serverpid; //pid of server
int numplayer; //indica se si tratta di player 1 o 2
int computer = 0;

int semid;
int shmid;
int fifoserver2clientfd;
int fifoclient2serverfd;
char buffer[100];

void sigIntHandler(int sig){
    printf("The signal ctrl-c was caught!\n");
}

void sigUsrHandler(int sig){
    
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
        ErrExit("Sintassi sbagliata\nLa sintassi corretta è: ./client nomegiocatore\n");

    if (strcmp(argv[1], "-c") == 0){
        computer = 1;
    }

    else{
        username = (char *)argv[1];
    }
    
    //blocco ctrl-c per rimuovere i semafori, la memoria condivisa e la fifo
    //blocco sigusr1 che mi comunica dal server se ho vinto il gioco, se ho perso

    if (signal(SIGTERM, sigIntHandler) == SIG_ERR ||
        signal(SIGUSR1, sigUsrHandler) == SIG_ERR)
        ErrExit("change signal handler failed");

    //apro il semaforo creato dal master
    semid = semget(SEM_KEY, 3 , S_IRUSR | S_IWUSR);

    if (semid == -1)
        ErrExit("Errore nella configurazione dei semafori (forse il server non è in esecuzione?)");

    //mi prendo la memoria condivisa
    shmid = alloc_shared_memory(SHM_KEY, sizeof(mymatrix));

    //apro la fifo client
    fifoclient2serverfd = open(fifoclient2serverpath, O_WRONLY);
    if (fifoclient2serverfd == -1)
        ErrExit("Errore in apertura fifoclient2server");

    fifoserver2clientfd = open(fifosever2clientpath, O_RDONLY);
    if (fifoserver2clientfd == -1)
        ErrExit("Errore in apertura fifoserver2client");

    int br = read(fifoserver2clientfd, &buffer, sizeof(buffer));
    if (br == -1)
        ErrExit("read fifoserver2clientfd fallita");
    else if (br != strlen(buffer))
        ErrExit("read fifoserver2clientfd sbagliata");


    serverpid = (pid_t) atoi(&buffer[1]);

    numplayer = buffer[0] - 48; 

    if (numplayer == 1)
        printf("In attesa di player 2");

    itoa(getpid(), buffer);

    br = write(fifoclient2serverfd, &buffer, sizeof(buffer));

    semOp(semid, 0, 1);
    
    return 0;
}
