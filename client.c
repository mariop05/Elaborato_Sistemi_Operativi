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
#include "utils.h"
#include <sys/msg.h>

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
struct mymsg msg1, msg2;
void sigHandler(int sig){
    if(sig == SIGUSR1){
        printf("inizio gioco\n");
    }
    else if(sig == SIGINT){
//        printf("Giocatore 1 ha abbandonato la partita.\nHai perso, Giocatore 2 ha vinto\n");
        if(numplayer == 0) {
            kill(serverpid, SIGUSR1);
        }
        else if(numplayer == 1){
            kill(serverpid, SIGUSR2);
        }
    } else if(sig == SIGUSR2){
        //giocatore 1
        if(numplayer == 0){
            if(msgrcv(msqid, &msg1, msgsize, 1, 0) == -1){
                ErrExit("msgrcv failed");
            }
            printf("\n%s\n", msg1.mtext);
            _exit(0);
        }
        //giocatore 2
        else if(numplayer == 1){
            printmatrix(mymatrix);
            if(msgrcv(msqid, &msg1, msgsize, 2, 0) == -1){
                ErrExit("msgrcv failed");
            }
            printf("\n%s\n", msg1.mtext);
            _exit(0);
        }

    }
}

int main(int argc, char const *argv[])
{
    struct mymsg message;
    size_t mSize = sizeof(message) - sizeof(long);

    printf("ESECUZIONE CLIENT\n");

    if(signal(SIGINT, sigHandler) == SIG_ERR || signal(SIGUSR1, sigHandler) == SIG_ERR ||
            signal(SIGUSR2, sigHandler) == SIG_ERR){
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
    else {
        ErrExit("Sintassi sbagliata\nLa sintassi corretta è: ./client nomegiocatore\nPer giocare contro il computer è necessario inserire * come secondo parametro");
    }

    //apro coda di messaggi
    msqid = msgget(MSQ_KEY, S_IRUSR | S_IWUSR);
    if(msqid == -1){
        ErrExit("msgget failed");
    }

    //apro il semaforo creato dal master
    semid = semget(SEM_KEY, 2 , S_IRUSR | S_IWUSR);

    if (semid == -1)
        ErrExit("Errore nella configurazione dei semafori (forse il server non è in esecuzione?)");

    //creo un segmento di memoria condivisa
    shmid = alloc_shared_memory(SHM_KEY, sizeof(matrix));
    //printf("shmid: %d\n", shmid);

    //attach the shared memory in read/write mode
    mymatrix = (matrix *)get_shared_memory(shmid, 0);



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

    //SE GIOCATORE 1 ATTENDE GIOCATORE 2
    if(numplayer == 0) {
        printf("In attesa di giocatore 2\n");
        closeFifo(fifoclient2serverfd);
        closeFifo(fifoserver2clientfd);
        pause();
    }
    // QUANDO SI CONNETTE GIOCATORE DUE STAMPA A VIDEO CHE IL GIOCO INIZIERÀ
    else{
//        printf("inizio gioco\n");
        closeFifo(fifoclient2serverfd);
        closeFifo(fifoserver2clientfd);
    }

    fflush(stdout);
    //esecuzione del giocatore
    while (1){
        bool flag = false;
        //sblocco primo giocatore
        semOp(semid, numplayer, -1);
        fflush(stdout);
        printmatrix(mymatrix);
        int c; //colonna dove viene inserito il gettone
        int ris = 0;
        char num;
        do{
            //inserisco la colonna
            printf("Inserisci il numero della colonna: ");
            scanf("%d", &c);
            char n = c + 48;
            //verifico se è stato inserito un numero
            if(n < 48 || n > 57){
                ris = -1;
            }
            if(c >= mymatrix->length){
                ris = -1;
            } else{
                ris = insert(mymatrix, mysymbol, c);
            }
        }while(ris != 0);
        printmatrix(mymatrix);
        semOp(semid, numplayer, -1);
    }


    return 0;
}