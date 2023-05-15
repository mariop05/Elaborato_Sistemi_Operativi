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
#include <sys/msg.h>
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
int fifoServer2ClientFd, fifoClient2ServerFd;
struct mymsg msg1;
void creazioneFifo(char *path1, char *path2);
void chiusuraFifo(int fd1, int fd2);
void eliminazioneFifo(char *path1, char *path2);
int estraggoPidClient(char buffer[]);
void fineSequenza();

int main(int argc, char *argv[]) {

    char *matrice;
    char buffer[100];
    int turno = 0;
    int vincita;
    char *textMessageQueue;

    // set the function sigHandler as handler for the signal SIGINT
    if (signal(SIGINT, sigHandler) == SIG_ERR || signal(SIGUSR1, sigHandler) == SIG_ERR ||
            signal(SIGUSR2, sigHandler) == SIG_ERR)
        return -1;

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

    msqid = msgget(MSQ_KEY, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(msqid == -1) {
        printf("msgget failed");
        return -1;
    }

    //creating new semaphore
    semid = semget(SEM_KEY, 2, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(semid == -1){
        ErrExit("semget failed");
    }
    //imposto i semafori con i giusti valori
    union semun arg;
    unsigned short values[] = {0,0};
    arg.array = values;

    if (semctl(semid, 0/*ignored*/, SETALL, arg) == -1)
        ErrExit("semctl SETALL");



    //creo un segmento di memoria condivisa
    shmid = alloc_shared_memory(SHM_KEY, sizeof(matrix));
    //printf("shmid: %d\n", shmid);

    //attach the shared memory in read/write mode
    mymatrix = (matrix *)get_shared_memory(shmid, 0);
    mymatrix->heigth = riga;
    mymatrix->length = colonna;

    //inizializza la matrice
    initializematrix(mymatrix);

//    //stampa matrice
//    printmatrix(mymatrix);

    //create new fifo
    creazioneFifo(fifoserver2clientpath, fifoclient2serverpath);
    //opening fifo
    fifoServer2ClientFd = openFifo(fifoserver2clientpath, O_WRONLY);
    fifoClient2ServerFd = openFifo(fifoclient2serverpath, O_RDONLY);

    //salvo il pid del server
    pid_t pidServer = getpid();


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
    chiusuraFifo(fifoServer2ClientFd, fifoClient2ServerFd);

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

    //converto da string ad int
    client2 = atoi(&buffer[index]);

    //closing fifo
    chiusuraFifo(fifoServer2ClientFd, fifoClient2ServerFd);

    //removing fifo
    eliminazioneFifo(fifoserver2clientpath, fifoclient2serverpath);


//    //invio a giocatore 1 che giocatore 2 è stato trovato
    kill(client1, SIGUSR1);


    while (1){
        //turno è pari --> giocatore 1 ha vinto
        //turno è dispari --> giocatore 2 ha vinto
        printf("turno: %d\n", turno);
        //aspetto che il giocatore faccia la sua mossa
        semOp(semid, turno % 2, 2);
        semOp(semid, turno % 2, 0);
        //verifico la vittoria
        int win = checkwin(mymatrix, 0);

        if(win == 1){
            //VITTORIA GIOCATORE 1
            if(turno % 2 == 0){
                msg1.mtype = 1;
                textMessageQueue = "HAI VINTO\n";
                memcpy(msg1.mtext, textMessageQueue, strlen(textMessageQueue) + 1);
                size_t mSize = sizeof(msg1) - sizeof(long);
                if(msgsnd(msqid, &msg1, mSize, 0) == -1){
                    ErrExit("msgsnd failed");
                }
                msg1.mtype = 2;
                textMessageQueue = "HAI PERSO\n";
                memcpy(msg1.mtext, textMessageQueue, strlen(textMessageQueue) + 1);
                mSize = sizeof(msg1) - sizeof(long);
                if(msgsnd(msqid, &msg1, mSize, 0) == -1){
                    ErrExit("msgsnd failed");
                }
                kill(client1, SIGUSR2);
                kill(client2, SIGUSR2);
            }
            //VITTORIA GIOCATORE 2
            else if(turno % 2 != 0){
                msg1.mtype = 1;
                textMessageQueue = "HAI PERSO\n";
                memcpy(msg1.mtext, textMessageQueue, strlen(textMessageQueue) + 1);
                size_t mSize = sizeof(msg1) - sizeof(long);
                if(msgsnd(msqid, &msg1, mSize, 0) == -1){
                    ErrExit("msgsnd failed");
                }
                msg1.mtype = 2;
                textMessageQueue = "HAI VINTO\n";
                memcpy(msg1.mtext, textMessageQueue, strlen(textMessageQueue) + 1);
                mSize = sizeof(msg1) - sizeof(long);
                if(msgsnd(msqid, &msg1, mSize, 0) == -1){
                    ErrExit("msgsnd failed");
                }
                kill(client1, SIGUSR2);
                kill(client2, SIGUSR2);
            }
            //ELIMINAZIONE DI SHARED MEMORY, MESSAGE QUEUE, FIFO
            fineSequenza();
        }
        else if(win == 2){
            msg1.mtype = 1;
            textMessageQueue = "Partita finita in pareggio\n";
            memcpy(msg1.mtext, textMessageQueue, strlen(textMessageQueue) + 1);
            size_t mSize = sizeof(msg1) - sizeof(long);
            if(msgsnd(msqid, &msg1, mSize, 0) == -1){
                ErrExit("msgsnd failed");
            }
            msg1.mtype = 2;
            memcpy(msg1.mtext, textMessageQueue, strlen(textMessageQueue) + 1);
            if(msgsnd(msqid, &msg1, mSize, 0) == -1){
                ErrExit("msgsnd failed");
            }
            kill(client1, SIGUSR2);
            kill(client2, SIGUSR2);
            fineSequenza();
        }
        turno++;
    }




    return 0;

}

//Stampo a video un avvertimento quando viene premuto una volta CTRL+C
void sigHandler(int sig) {
    if(sig == SIGINT){
        count = count + 1;
        if(count == 1)
            printf("Una seconda pressione di CTRL+C comporterà la terminazione del gioco.\n");
        else if(count == 2){
            kill(client1, SIGTERM);
            kill(client2, SIGTERM);
            fineSequenza();
        }
    }
    //abbandono giocatore 1
    if(sig == SIGUSR1){
        printf("ricevuto segnale SIGUSR1\n");
        msg1.mtype = 1;
        char *textMessageQueue = "Hai perso per abbandono\n";
        memcpy(msg1.mtext, textMessageQueue, strlen(textMessageQueue) + 1);
//        printf("msg1.text: %s\n", msg1.mtext);
        size_t mSize = sizeof(msg1) - sizeof(long);
        if(msgsnd(msqid, &msg1, mSize, 0) == -1){
            ErrExit("msgsnd failed");
        }
        msg1.mtype = 2;
        char *textMessageQueue1 = "Hai vinto per abbandono\n";
        memcpy(msg1.mtext, textMessageQueue1, strlen(textMessageQueue1) + 1);
//        printf("msg1.text: %s\n", msg1.mtext);
        if(msgsnd(msqid, &msg1, mSize, 0) == -1){
            ErrExit("msgsnd failed");
        }
        kill(client1, SIGUSR2);
        kill(client2, SIGUSR2);
        fineSequenza();
    }
    //abbandono giocatore 2
    else if(sig == SIGUSR2){
        printf("ricevuto segnale SIGUSR2\n");
        msg1.mtype = 1;
        char *textMessageQueue = "Hai vinto per abbandono\n";
        memcpy(msg1.mtext, textMessageQueue, strlen(textMessageQueue) + 1);
//        printf("msg1.text: %s\n", msg1.mtext);
        size_t mSize = sizeof(msg1) - sizeof(long);
        if(msgsnd(msqid, &msg1, mSize, 0) == -1){
            ErrExit("msgsnd failed");
        }
        msg1.mtype = 2;
        char *textMessageQueue1 = "Hai perso per abbandono\n";
        memcpy(msg1.mtext, textMessageQueue1, strlen(textMessageQueue1) + 1);
//        printf("msg1.text: %s\n", msg1.mtext);
        if(msgsnd(msqid, &msg1, mSize, 0) == -1){
            ErrExit("msgsnd failed");
        }
        kill(client1, SIGUSR2);
        kill(client2, SIGUSR2);
        fineSequenza();
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
void creazioneFifo(char *path1, char *path2){
    //create new fifo
    createFifo(path1,  S_IRUSR | S_IWUSR);
    createFifo(path2,  S_IRUSR | S_IWUSR);
}
void chiusuraFifo(int fd1, int fd2){
    closeFifo(fd1);
    closeFifo(fd2);
}
void eliminazioneFifo(char *path1, char *path2){
    //removing fifo
    removeFifo(path1);
    removeFifo(path2);
}
void fineSequenza(){
    //closing fifo
    chiusuraFifo(fifoServer2ClientFd, fifoClient2ServerFd);
    //removing fifo
    eliminazioneFifo(fifoserver2clientpath, fifoclient2serverpath);
    //detach a segment of shared memory
    free_shared_memory(mymatrix);
    // delete the shared memory segment
    remove_shared_memory(shmid);
    if (msgctl(msqid, IPC_RMID, NULL) == -1)
        ErrExit("msgctl failed");
    _exit(0);

}
