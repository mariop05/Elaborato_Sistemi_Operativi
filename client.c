#include <stdio.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "message_queue.h"
#include "err_exit.h"
#include "matrix.h"
#include "semaphore.h"
#include <unistd.h>
#include "shared_memory.h"
#include "fifo.h"

#define SEM_KEY 10
#define SHM_KEY 11
#define MSQ_KEY 12
#define ALARM_TIME 30

char *fifoclient2serverpath = "./client2server";
char *fifosever2clientpath = "./server2client";

const size_t msgsize = sizeof(struct mymsg) - sizeof(long);

char *username; //the client username
char mysymbol;
matrix *mymatrix; //the client matrix pointer
struct mymsg msg;
pid_t serverpid; //pid of server
int numplayer; //indica se si tratta di player 1 o 2 (0,1)
int computer = 0;

int semid;
int shmid;
int fifoserver2clientfd;
int fifoclient2serverfd;
char buffer[100];
int msqid;
void sigHandlerSIGUSR1(int sig){
    if(sig == SIGUSR1){
        printf("inizio gioco");
    }
}

int main(int argc, char const *argv[])
{
    printf("ESECUZIONE CLIENT\n");
    printf("PID GIOCATORE: %d\n", getpid());

    if(signal(SIGUSR1, sigHandlerSIGUSR1) == SIG_ERR){
        ErrExit("signal failed");
    }



    /*
     * Verifico se il giocatore inserisce due o tre argomenti.
     * Se gli argomenti inseriti sono 2 allora viene inserito solo il nome del giocatore
     * Se gli argomenti inseriti sono 3 allora viene il giocatore gioca contro il computer
     */
    if (argc == 2)
        username = (char *)argv[1];
    else if (argc == 3 && strcmp(argv[2], "-c") == 0){
        username = (char *)argv[1];
        computer = 1;
    }
    else
        ErrExit("Sintassi sbagliata\nLa sintassi corretta è: ./client nomegiocatore\nPer giocare contro il computer è necessario inserire * come secondo parametro");


    //apro la fifo lato server
    fifoserver2clientfd = openFifo(fifosever2clientpath, O_RDONLY);

    //apro la fifo lato client
    fifoclient2serverfd = openFifo(fifoclient2serverpath, O_WRONLY);

    ssize_t br = read(fifoserver2clientfd, buffer, sizeof(buffer));
    if(br == -1) {
        ErrExit("read failed");
    }

    // salvo il numero del giocatore, il simbolo e il pid del server
    numplayer = buffer[0] - 49;
    mysymbol = buffer[1];
    serverpid = (pid_t) atoi(&buffer[2]);
    sprintf(buffer, "%s%d", username, getpid());
//    printf("buffer: %s\n", buffer);
    //invio a server le informazioni del client
    if(write(fifoclient2serverfd, buffer, sizeof(buffer)) == -1){
        ErrExit("write failed");
    }

    if(numplayer == 0) {
        printf("In attesa di giocatore 2\n");
        closeFifo(fifoclient2serverfd);
        closeFifo(fifoserver2clientfd);
        pause();
    }
    else{
        printf("inizio gioco\n");
        closeFifo(fifoclient2serverfd);
        closeFifo(fifoserver2clientfd);
    }


    return 0;
}