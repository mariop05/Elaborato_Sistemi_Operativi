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
char *fifoclient2serverpath = "../client2server";
char *fifosever2clientpath = "../server2client";
int count = 0;
int shmid;
pid_t client1, client2;
matrix mymatrix;
char *puntatoreSharedMemory;
int main(int argc, char *argv[]) {


    char *matrice;
    char buffer[10];

    // set the function sigHandler as handler for the signal SIGINT
    if (signal(SIGINT, sigHandler) == SIG_ERR)
        return -1;


    //    printf("argc: %d\n", argc);
    //verifico se il server riceve 5 input
    if(argc != 5) {
        ErrExit("Per lanciare correttamente il server bisongna inserire i seguenti parametri:\nRiga matrice, Colonna matrice, Forma gettone uno, Forma gettone due\nEsempio: ./server 5 5 X O\n");
    }



    //salvo il numero di righe e colonne inserite dall'utente
    int riga = conversioneCarattereInNumero(*argv[1]);
    int colonna = conversioneCarattereInNumero(*argv[2]);
    char simboloUno = *argv[3];
    char simboloDue = *argv[4];
    mymatrix.heigth = riga;
    mymatrix.length = colonna;

    //verifico che venga create almeno una matrice di dimensione 5x5
    if(riga < 5 && colonna < 5){
        ErrExit("Creare una matrice di dimensione almeno 5x5\n");
    }

    /** RICEVO PID DA GIOCATORE 1 **/
    //create new fifo
    int fifoClientToServer = createFifo(fifoclient2serverpath, S_IRUSR | S_IWUSR);
    //opening fifo
    int fifoClientToServerFd1 = openFifo(fifoclient2serverpath, O_RDONLY);
    if(read(fifoClientToServerFd1, buffer, sizeof(buffer)) == -1){
        ErrExit("error read");
    }
    printf("buffer: %s\n", buffer);
    closeFifo(fifoClientToServerFd1);

    /** RICEVO PID DA GIOCATORE 2 **/
    //opening fifo
    int fifoClientToServerFd2 = openFifo(fifoclient2serverpath, O_RDONLY);
    if(read(fifoClientToServerFd2, buffer, sizeof(buffer)) == -1){
        ErrExit("error read");
    }
    printf("buffer: %s\n", buffer);
    closeFifo(fifoClientToServerFd2);






    //creo un segmento di memoria condivisa
    shmid = alloc_shared_memory(shmid, sizeof(mymatrix));
    printf("shmid: %d\n", shmid);
    //attach the shared memory in read/write mode
    puntatoreSharedMemory = get_shared_memory(shmid, 0);
    //writing on shared memory
    for(int i = 0; i < mymatrix.heigth; i++){
        for(int j = 0; j < mymatrix.length; j++){
            puntatoreSharedMemory[i*colonna+j] = mymatrix.table[i][j];
        }
    }
    char *readSharedMemoryPtr = get_shared_memory(shmid, SHM_RDONLY);
    //reading from shared memory
    for(int i = 0; i < mymatrix.heigth; i++){
        for(int j = 0; j < mymatrix.length; j++){
            printf("%c ", readSharedMemoryPtr[i*colonna+j]);
        }
        printf("\n");
    }

//
//    //se CTRL+C viene premuto due volte il gioco si interrompe
//    while(count < 2){
//
//    };

    //detach a segment of shared memory
    free_shared_memory(puntatoreSharedMemory);
    // delete the shared memory segment
    remove_shared_memory(shmid);
    removeFifo(fifoclient2serverpath);



}
//Stampo a video un avvertimento quando viene premuto una volta CTRL+C
void sigHandler(int sig) {
    if(sig == SIGINT){
        count = count + 1;
        if(count == 1)
            printf("Una seconda pressione di CTRL+C comporterà la terminazione del gioco.\n");
        else if(count == 2){
            //detach a segment of shared memory
            free_shared_memory(puntatoreSharedMemory);
            // delete the shared memory segment
            remove_shared_memory(shmid);
            removeFifo(fifoclient2serverpath);
        }
    }
}
int conversioneCarattereInNumero(char c){
    // 0 -> 48
    // 1 -> 49...
    // 9 -> 57
    if(c == '0')
        return 0;
    if(c == '1')
        return 1;
    if(c == '2')
        return 2;
    if(c == '3')
        return 3;
    if(c == '4')
        return 4;
    if(c == '5')
        return 5;
    if(c == '6')
        return 6;
    if(c == '7')
        return 7;
    if(c == '8')
        return 8;
    if(c == '9')
        return 9;
    return -1;
}