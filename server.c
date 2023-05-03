#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
//librerie esterne
#include "err_exit.h"
#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "matrix.h"


void sigHandler(int sig);
int conversioneCarattereInNumero(char c);
int count = 0;
int shmid;
pid_t client1, client2;
matrix mymatrix;
char *puntatoreSharedMemory;
int main(int argc, char *argv[]) {


    char *fifoclient2serverpath = "/home/mario00/Scrivania/Progetto Sistemi Operativi/client2server";
    char *fifoserver2clientpath = "/home/mario00/Scrivania/Progetto Sistemi Operativi/server2client";
    char buffer[100];
    //    printf("argc: %d\n", argc);
    //verifico se il server riceve 5 input
    if(argc != 5) {
        ErrExit("Per lanciare correttamente il server bisongna inserire i seguenti parametri:\nRiga matrice, Colonna matrice, Forma gettone uno, Forma gettone due\nEsempio: ./server 5 5 X O\n");
    }

    /** RICEVO PID DA GIOCATORE 1 **/
    //creting new fifo
    int fifoClientToServer = createFifo(fifoclient2serverpath, S_IRUSR | S_IWUSR);
    //opening fifo
    int fd = openFifo(fifoclient2serverpath, O_RDONLY);
    if(read(fd, buffer, sizeof(buffer)) == -1){
        ErrExit("error read first PID");
    }
    printf("pid1: %s\n", buffer);
    closeFifo(fd);

    /** INVIO A GIOCATORE 1 AVVISO DI ATTESA RICERCA GIOCATORE DUE */
    char attesaBuffer[] = "Ricerca di giocatore 2";
    //opening fifo
    int fd2 = openFifo(fifoserver2clientpath, O_WRONLY);
    if(write(fd2, attesaBuffer, sizeof(attesaBuffer)) == -1){
        ErrExit("error first write");
    }
    closeFifo(fd2);


    /** RICEVO PID DA GIOCATORE 2 **/
    fd = openFifo(fifoclient2serverpath, O_RDONLY);
    if(read(fd, buffer, sizeof(buffer)) == -1){
        ErrExit("error read second PID");
    }
    printf("pid2: %s\n", buffer);
    closeFifo(fd);

//    /** AVVISO GIOCATORE 1 CHE È STATO TROVATO UN AVVERSARIO **/
//    char trovatoGiocatore[] = "Trovato giocatore 2";
//    //opening fifo
//    printf("opening fifo2");
//    fd2 = openFifo(fifoserver2clientpath, O_WRONLY);
//    if(write(fd2, trovatoGiocatore, sizeof(trovatoGiocatore)) == -1){
//        ErrExit("error first write");
//    }
//    closeFifo(fd2);



    //removing all fifos
    removeFifo(fifoclient2serverpath);







}