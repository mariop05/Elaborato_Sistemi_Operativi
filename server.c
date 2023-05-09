#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
//librerie esterne
#include "err_exit.h"
#include "fifo.h"
#include "semaphore.h"
#include "shared_memory.h"
#include "matrix.h"
#include "message_queue.h"

#define SEM_KEY 10
#define SHM_KEY 11
#define MSQ_KEY 12

void sigIntHandler(int sig);
void sigUsrHandler(int sig);


char *fifoclient2serverpath = "./client2server";
char *fifoserver2clientpath = "./server2client";

const size_t msgsize = sizeof(struct mymsg) - sizeof(long);

int count = 0;
int shmid;
int semid;
int msqid;
int computer = 0;
pid_t client1, client2;
matrix *mymatrix;

void exitSequence(){

    fflush(stdout);
    free_shared_memory(mymatrix);
    
    sleep(1);
    if (semctl(semid, 0, IPC_RMID) == -1){
        ErrExit("rimozione semafori fallita");
    }

    //struct msqid_ds msqidds;
    //if (msgctl(msqid, IPC_STAT, &msqidds) == -1)
    //    ErrExit("ricezione informazioni fallita");

    if (msgctl(msqid, IPC_RMID, NULL) == -1)
        //ErrExit("rimozione messaggi fallita");

    removeFifo(fifoserver2clientpath);
    removeFifo(fifoclient2serverpath);

    if(computer == 1){
        kill(client2, SIGTERM);
    }

    exit(0);

}

void sendwin(int wincode, int winner){

    //win
    if(wincode == 1){
        
        struct mymsg msg1, msg2;
        msg1.mtype = 1;
        msg2.mtype = 1;
        sprintf(msg1.mtext, "%dwin", winner);
        sprintf(msg2.mtext, "%dwin", winner);

        msgsnd(msqid, &msg1, msgsize, 0);
        if(computer == 0){
            msgsnd(msqid, &msg2, msgsize, 0);
        }
        
        kill(client1, SIGUSR1);
        if(computer == 0)
            kill(client2, SIGUSR1);

        exitSequence();
    }
    
    //parity
    else if(wincode == 2){
        struct mymsg msg1, msg2;
        msg1.mtype = 1;
        msg2.mtype = 1;
        sprintf(msg1.mtext, "parity");
        sprintf(msg2.mtext, "parity");

        msgsnd(msqid, &msg1, msgsize, 0);
        if(computer == 0){
            msgsnd(msqid, &msg2, msgsize, 0);
        }

        kill(client1, SIGUSR1);
        if(computer == 0)
            kill(client2, SIGUSR1);

        exitSequence();
    }
    //win for exit
    else if(wincode == 3){
        struct mymsg msg1;
        msg1.mtype = 1;
        sprintf(msg1.mtext, "winforctrlc");

        msgsnd(msqid, &msg1, msgsize, 0);

        if (winner == 0)
            kill(client1, SIGUSR1);
        
        else if (winner == 1)
            kill(client2, SIGUSR1);

        exitSequence();
    }
}

void computerGame(char mysymbol){

    close(STDOUT_FILENO);
    srand(time(NULL));

    while(1){

        semOp(semid, 1, -1);
        int column = 0;

        do{
            column = rand() % mymatrix->length + 1;
        }
        while(insert(mymatrix, mysymbol, column - 1));

        semOp(semid, 1, -1);
    }
}

int main(int argc, char *argv[]) {

    char *matrice;
    char buffer[10];

    // set the function sigHandler as handler for the signal SIGINT
    if (signal(SIGINT, sigIntHandler) == SIG_ERR ||
        signal(SIGUSR1,sigUsrHandler) == SIG_ERR)
        ErrExit("signal failed");

    //    printf("argc: %d\n", argc);
    //verifico se il server riceve 5 input
    if(argc != 5) {
        ErrExit("Per lanciare correttamente il server bisongna inserire i seguenti parametri:\nRiga matrice, Colonna matrice, Forma gettone uno, Forma gettone due\nEsempio: ./server 5 5 X O\n");
    }

    //salvo il numero di righe e colonne inserite dall'utente
    int riga = atoi(argv[1]);
    int colonna = atoi(argv[2]);
    char simboloUno = *argv[3];
    char simboloDue = *argv[4];

    //verifico che venga create almeno una matrice di dimensione 5x5
    if(riga < 5 && colonna < 5){
        ErrExit("Creare una matrice di dimensione almeno 5x5\n");
    }

    printf("Server in esecuzione\n");

     //creo un segmento di memoria condivisa
    shmid = alloc_shared_memory(SHM_KEY, sizeof(matrix));

    //attach the shared memory in read/write mode
    mymatrix = (matrix *)get_shared_memory(shmid, 0);
    mymatrix->heigth = riga;
    mymatrix->length = colonna;

    //inizializza la matrice
    initializematrix(mymatrix);

    //apro la coda di messaggi creata dal master
    msqid = msgget(MSQ_KEY, IPC_CREAT | S_IRUSR | S_IWUSR);

    if (msqid == -1)
        ErrExit("msgget failed");

    //apro un nuovo semaforo
    semid = semget(SEM_KEY, 2 ,IPC_CREAT |  S_IRUSR | S_IWUSR);

    if (semid == -1)
        ErrExit("semget failed");

    //imposto i semafori con i giusti valori
    union semun arg;
    unsigned short values[] = {0,0};
    arg.array = values;

    if (semctl(semid, 0/*ignored*/, SETALL, arg) == -1)
        ErrExit("semctl SETALL");

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
    //else if (br != strlen(buffer))
        //ErrExit("write server2client sbagliata");

    /** RICEVO PID DA GIOCATORE 1 **/

    if(read(fifoClient2ServerFd, buffer, sizeof(buffer)) == -1){
        ErrExit("error read");
    }

    //il primo numero decide se giocherò contro il pc
    computer = buffer[0] - 48;

    printf("Modalità di gioco: %s\n", computer == 0? "due giocatori" : "un giocatore");

    // memorizzo il pid del server
    client1 = (pid_t) atoi(&buffer[1]);

    closeFifo(fifoServer2ClientFd);
    closeFifo(fifoClient2ServerFd);

    sleep(1);

    // se gioco contro il pc non vado ad interrogare il secondo client
    if (computer == 0)
    {
        //opening fifo
        fifoServer2ClientFd = openFifo(fifoserver2clientpath, O_WRONLY);
        fifoClient2ServerFd = openFifo(fifoclient2serverpath, O_RDONLY);

        /** INVIO IL MIO PID A GIOCATORE 1 **/

        sprintf(buffer, "%d%c%d", 2, simboloDue, getpid());

        int br = write(fifoServer2ClientFd, buffer, sizeof(buffer));
        if (br == -1)
            ErrExit("write fifo server2client fallita");
        //else if (br != strlen(buffer))
        //ErrExit("write server2client sbagliata");
        
        /** RICEVO PID DA GIOCATORE 2 **/
        //opening fifo
        
        if(read(fifoClient2ServerFd, buffer, sizeof(buffer)) == -1){
            ErrExit("error read");
        }

        client2 = (pid_t) atoi(&buffer[1]);

        closeFifo(fifoServer2ClientFd);
        closeFifo(fifoClient2ServerFd);

    }

    else if (computer == 1)
    {
        pid_t client2 = fork();
        
        if (client2 == -1){
            ErrExit("error fork");
        }

        else if(client2 == 0){
            computerGame(simboloDue);
        }
    }

    int turn = 0;
    int win = 0;
    while(1)
    {
        //aspetto che il giocatore faccia la sua mossa
        semOp(semid, turn % 2, 2);
        semOp(semid, turn % 2, 0);

        //guardo se il giocatore ha vinto
        win = checkwin(mymatrix,0);

        sendwin(win, turn % 2);

        turn ++;
    }
}
//Stampo a video un avvertimento quando viene premuto una volta CTRL+C
void sigIntHandler(int sig) {
    if(sig == SIGINT){
        count = count + 1;
        if(count == 1)
            printf("Una seconda pressione di CTRL+C comporterà la terminazione del gioco.\n");
        else if(count == 2){
            
            struct mymsg msg1, msg2;
            msg1.mtype = 1;
            msg2.mtype = 1;
            sprintf(msg1.mtext, "ctrlc");
            sprintf(msg2.mtext, "ctrlc");

            msgsnd(msqid, &msg1, msgsize, 0);
            if(computer == 0){
                msgsnd(msqid, &msg2, msgsize, 0);
            }
        
            kill(client1, SIGUSR1);
            if(computer == 0){
                kill(client2, SIGUSR1);
            }

            exitSequence();
        }
    }
}

void sigUsrHandler(int sig) {

    if(sig == SIGUSR1){

        struct mymsg msg;
        msg.mtype = 1;

        if (msgrcv(msqid, &msg, msgsize, 1, 0) == -1)
        ErrExit("Impossibile ricevere il messaggio msgrcv");

        char *message = msg.mtext;

        if(strcmp(&message[1], "ctrlc") == 0){

            printf("Giocatore %c ha abbandonato la partita", message[0]);

            int winner = (message[0] + 1 ) % 2;
            sendwin(3, winner);
        }
    }
}