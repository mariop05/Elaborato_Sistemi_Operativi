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
int conversioneCarattereInNumero(char c);
char *fifoclient2serverpath = "./client2server";
char *fifoserver2clientpath = "./server2client";
int count = 0;
int shmid;
int semid;
int msqid;
int computer = 0;
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

    //apro un nuovo semaforo
    semid = semget(SEM_KEY, 2 ,IPC_CREAT |  S_IRUSR | S_IWUSR);

    if (semid == -1)
        ErrExit("semget failed");

    //imposto i semafori con i giusti valori
    union semun arg;
    unsigned short values[] = {2,2};
    arg.array = values;

    if (semctl(semid, 0/*ignored*/, SETALL, arg) == -1)
        ErrExit("semctl SETALL");

    //creo un segmento di memoria condivisa
    shmid = alloc_shared_memory(SHM_KEY, sizeof(mymatrix));
    printf("shmid: %d\n", shmid);
    //attach the shared memory in read/write mode
    puntatoreSharedMemory = get_shared_memory(shmid, 0);

    //create new fifo
    createFifo(fifoserver2clientpath,  S_IRUSR | S_IWUSR);
    createFifo(fifoclient2serverpath,  S_IRUSR | S_IWUSR);

    //opening fifo
    int fifoServer2ClientFd = openFifo(fifoserver2clientpath, O_WRONLY);
    int fifoClient2ServerFd = openFifo(fifoclient2serverpath, O_RDONLY);

    /** INVIO IL MIO PID A GIOCATORE 1 **/

    sprintf(buffer, "%d%c%d", 1,simboloUno, getpid());

    int br = write(fifoServer2ClientFd, buffer, sizeof(buffer));
    if (br == -1)
        ErrExit("write fifo server2client fallita");
    else if (br != strlen(buffer))
        ErrExit("write server2client sbagliata");

    /** RICEVO PID DA GIOCATORE 1 **/

    if(read(fifoClient2ServerFd, buffer, sizeof(buffer)) == -1){
        ErrExit("error read");
    }

    printf("buffer: %s\n", buffer);

    //il primo numero decide se giocherò contro il pc
    computer = buffer[0] - 48;

    // memorizzo il pid del server
    client1 = (pid_t) atoi(&buffer[1]);

    closeFifo(fifoServer2ClientFd);
    closeFifo(fifoClient2ServerFd);

    // se gioco contro il pc non vado ad interrogare il secondo client
    if (computer == 0)
    {
        //opening fifo
        fifoServer2ClientFd = openFifo(fifoserver2clientpath, O_WRONLY);
        fifoClient2ServerFd = openFifo(fifoclient2serverpath, O_RDONLY);

        /** INVIO IL MIO PID A GIOCATORE 1 **/

        sprintf(buffer, "%d%c%d", 1,simboloUno, getpid());

        int br = write(fifoServer2ClientFd, buffer, sizeof(buffer));
        if (br == -1)
            ErrExit("write fifo server2client fallita");
        else if (br != strlen(buffer))
        ErrExit("write server2client sbagliata");
        
        /** RICEVO PID DA GIOCATORE 2 **/
        //opening fifo
        
        if(read(fifoClient2ServerFd, buffer, sizeof(buffer)) == -1){
            ErrExit("error read");
        }
        printf("buffer: %s\n", buffer);
    }

    closeFifo(fifoServer2ClientFd);
    closeFifo(fifoClient2ServerFd);

    //creo un segmento di memoria condivisa
    shmid = alloc_shared_memory(SHM_KEY, sizeof(mymatrix));
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