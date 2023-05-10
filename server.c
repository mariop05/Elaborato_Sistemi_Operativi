#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <string.h>
//librerie esterne
#include "err_exit.h"
#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "matrix.h"

#define SEM_KEY 10
#define SHM_KEY 11
#define MSQ_KEY 12

void sigHandler(int sig);
char *fifoclient2serverpath = "./client2server";
char *fifoserver2clientpath = "./server2client";
int count = 0;
int shmid;
int semid;
int msqid;
int computer = 0;
pid_t client1, client2;
matrix *mymatrix;

int estraggoPidClient(char buffer[]);

int main(int argc, char *argv[]) {

    char *matrice;
    char buffer[100];

    // set the function sigHandler as handler for the signal SIGINT
    if (signal(SIGINT, sigHandler) == SIG_ERR)
        return -1;

    //    printf("argc: %d\n", argc);
    //verifico se il server riceve 5 input
    if(argc != 5) {
        ErrExit("Per lanciare correttamente il server bisongna inserire i seguenti parametri:\nRiga matrice, Colonna matrice, Forma gettone uno, Forma gettone due\nEsempio: ./server 5 5 X O\n");
    }

    //salvo il numero di righe e colonne inserite dall'utente
    int riga = atoi(argv[1]);
    int colonna = atoi(argv[2]);
//    printf("riga: %d colonna: %d\n", riga, colonna);
    char simboloUno = *argv[3];
    char simboloDue = *argv[4];

    //verifico che venga create almeno una matrice di dimensione 5x5
    if(riga < 5 && colonna < 5){
        ErrExit("Creare una matrice di dimensione almeno 5x5\n");
    }

    //creo un segmento di memoria condivisa
    shmid = alloc_shared_memory(SHM_KEY, sizeof(matrix));
//    printf("shmid: %d\n", shmid);

    //attach the shared memory in read/write mode
    mymatrix = (matrix *)get_shared_memory(shmid, 0);
    mymatrix->heigth = riga;
    mymatrix->length = colonna;

    //inizializza la matrice
    initializematrix(mymatrix);

    //create new fifo
    createFifo(fifoserver2clientpath,  S_IRUSR | S_IWUSR);
    createFifo(fifoclient2serverpath,  S_IRUSR | S_IWUSR);

    //opening fifo
    int fifoServer2ClientFd = openFifo(fifoserver2clientpath, O_WRONLY);
    int fifoClient2ServerFd = openFifo(fifoclient2serverpath, O_RDONLY);

    //salvo il pid del server
    pid_t pidServer = getpid();

    //---------------------------------
    //GIOCATORE 1

    //trasformo in stringa il numero giocatore, simbolo, e il pid del server
    sprintf(buffer, "%d%c%d", 1, simboloUno, pidServer);
//    printf("buffer da server a client: %s\n", buffer);

    //SERVER INVIA A GIOCATORE 1 nomegiocatore|simbolo|pidServer
    if(write(fifoServer2ClientFd, buffer, sizeof(buffer)) == -1){
        ErrExit("write failed");
    }

    //SERVER SI METTE IN ATTESA CHE GIOCATORE 1 RISPONDA
    if(read(fifoClient2ServerFd, buffer, sizeof(buffer)) == -1){
        ErrExit("read failed");
    }
//    printf("Risposta giocatore 1: %s\n", buffer);

    int index = estraggoPidClient(buffer);
    client1 = atoi(&buffer[index]);
//    printf("client 1: %d\n", client1);


    //closing fifo
    closeFifo(fifoServer2ClientFd);
    closeFifo(fifoClient2ServerFd);

    //------------------------------------------
    //GIOCATORE 2

    fifoServer2ClientFd = openFifo(fifoserver2clientpath, O_WRONLY);
    fifoClient2ServerFd = openFifo(fifoclient2serverpath, O_RDONLY);

    sprintf(buffer, "%d%c%d", 2, simboloDue, pidServer);
//    printf("buffer da server a client: %s\n", buffer);

    //SERVER INVIA A GIOCATORE 1 nomegiocatore|simbolo|pidServer
    if(write(fifoServer2ClientFd, buffer, sizeof(buffer)) == -1){
        ErrExit("write failed");
    }

    //SERVER SI METTE IN ATTESA CHE GIOCATORE 1 RISPONDA
    if(read(fifoClient2ServerFd, buffer, sizeof(buffer)) == -1){
        ErrExit("read failed");
    }
    index = estraggoPidClient(buffer);
    client2 = atoi(&buffer[index]);


    kill(client1, SIGUSR1);




    //closing fifo
    closeFifo(fifoServer2ClientFd);
    closeFifo(fifoClient2ServerFd);


    //removing fifo
    removeFifo(fifoserver2clientpath);
    removeFifo(fifoclient2serverpath);

    return 0;

}

//Stampo a video un avvertimento quando viene premuto una volta CTRL+C
void sigHandler(int sig) {
    if(sig == SIGINT){
        count = count + 1;
        if(count == 1)
            printf("Una seconda pressione di CTRL+C comporterà la terminazione del gioco.\n");
        else if(count == 2){
            //detach a segment of shared memory
            free_shared_memory(mymatrix);
            // delete the shared memory segment
            remove_shared_memory(shmid);
        }
    }
}
int estraggoPidClient(char buffer[]){
    int flag = 1;
    for(int i = 0; i < strlen(buffer) && flag; i++){
        if(buffer[i] < 59) {
            flag = 0;
            return i;
        }
    }
    return 0;
}
