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

void exitSequence(){

    usleep(200);
    fflush(stdout);
    free_shared_memory(mymatrix);

    msgctl(msqid, IPC_RMID, NULL);

    exit(0);

}

void cicle(){

    while(1){
        // aspetto che il server mi sblocchi
        semOp(semid, numplayer, -1);

        system("clear"); 
        printmatrix(mymatrix);
        fflush(stdout);
        
        // imposto l' allarme tra X secondi
        printf("\nInserisci la cella in cui inserire il gettone entro %d secondi\n", ALARM_TIME);
        alarm(ALARM_TIME);

        int column;

        do{
            scanf("%d", &column);
        }
        while(insert(mymatrix, mysymbol, column - 1));

        alarm(0);

        system("clear");
        printmatrix(mymatrix);
        fflush(stdout);

        semOp(semid, numplayer, -1);
    }
}

void sigIntHandler(int sig) {

    printf("CTRL-C rilevato!\n");

    msg.mtype = 1;

    sprintf(msg.mtext, "%dctrlc", numplayer);

    if(msgsnd(msqid, &msg, msgsize, 0) == -1)
        ErrExit("Impossibile inviare il messaggio msgsnd");

    if(kill(serverpid, SIGUSR1) == -1) 
        ErrExit("Impossibile mandare il segnale al server");

    exitSequence();
}

void sigUsrHandler(int sig){

    if (msgrcv(msqid, &msg, msgsize, 1, 0) == -1)
        ErrExit("Impossibile ricevere il messaggio msgrcv");

    char *message = msg.mtext;

    if(strcmp(&message[1], "win") == 0){

        if(message[0] - 48 == numplayer){
            printf("\nComplimenti, HAI VINTO!!!\n");
        }
        else{
            printf("\nMi dispiace HAI PERSO :(\n");
        }
        exitSequence();
    }

    else if(strcmp(message, "ctrlc") == 0){

        printf("\nIl server ha abbandonato la partita\n");
        exitSequence();
    }

    else if(strcmp(message, "winforctrlc") == 0){

        printf("\nHAI VINTO per abbandono dell' avversario\n");
        exitSequence();
    }

    else if(strcmp(message, "parity") == 0){

        printf("\nPARITA\n'");
        exitSequence();
    }

    else {

        printf("\nAttenzione: messaggio non gestito\n");
    }

}

void sigAlaHandler(int sig){
    
    if (sig == SIGALRM)
    {
        printf("Tempo scaduto, ora tocca all' avversario\n");
        fflush(stdout);
        semOp(semid, numplayer, -1);
        cicle();
    }
    
}

int main(int argc, char const *argv[])
{
    if (argc == 2)
        username = (char *)argv[1];
        

    else if (argc == 3 && strcmp(argv[2], "*") == 0){
        username = (char *)argv[1];
        computer = 1;
    }

    else
       ErrExit("Sintassi sbagliata\nLa sintassi corretta è: ./client nomegiocatore\nPer giocare contro il computer è necessario inserire * come secondo parametro");
    
    //blocco ctrl-c per rimuovere i semafori, la memoria condivisa e la fifo
    //blocco sigusr1 che mi comunica dal server se ho vinto il gioco, se ho perso

    if (signal(SIGINT, sigIntHandler) == SIG_ERR ||
        signal(SIGUSR1, sigUsrHandler) == SIG_ERR ||
        signal(SIGALRM, sigAlaHandler) == SIG_ERR)
        ErrExit("change signal handler failed");

    //apro il semaforo creato dal master
    semid = semget(SEM_KEY, 2 , S_IRUSR | S_IWUSR);

    if (semid == -1)
        ErrExit("Errore nella configurazione dei semafori (forse il server non è in esecuzione?)");

    //apro la coda di messaggi creata dal master
    msqid = msgget(MSQ_KEY, S_IRUSR | S_IWUSR);

    if (semid == -1)
        ErrExit("Errore nella configurazione dei messaggi (forse il server non è in esecuzione?)");

    //mi prendo la memoria condivisa
    shmid = take_shared_memory(SHM_KEY, sizeof(matrix));
    mymatrix = get_shared_memory(shmid, 0);

    //apro la fifo server
    fifoserver2clientfd = openFifo(fifosever2clientpath, O_RDONLY);

    //apro la fifo client
    fifoclient2serverfd = openFifo(fifoclient2serverpath, O_WRONLY);

    int br = read(fifoserver2clientfd, buffer, sizeof(buffer));

    if (br == -1)
        ErrExit("read fifo server2client fallita");

    //else if (br != strlen(buffer))
        //ErrExit("read fifo server2client sbagliata");
    printf("\n%s\n" , buffer);

    // mi permette di capire se sono player 1 o 2
    numplayer = buffer[0] - 49;

    // memorizzo il simbolo del giocatore
    mysymbol = buffer[1];

    // memorizzo il pid del server
    serverpid = (pid_t) atoi(&buffer[2]);

    if (numplayer == 0 && computer == 0)
    {
        printf("In attesa di player 2\n");
        fflush(stdout);
    }

    sprintf(buffer, "%c%d", computer + 48, getpid());

    br = write(fifoclient2serverfd, buffer, sizeof(buffer));
    if (br == -1)
        ErrExit("write fifo client2server fallita");
    //else if (br != strlen(buffer))
        //ErrExit("write client2server sbagliata");

    closeFifo(fifoclient2serverfd);
    closeFifo(fifoserver2clientfd);
    
    //entro nel loop del gioco
    cicle();
    
    
    return 0;
}
