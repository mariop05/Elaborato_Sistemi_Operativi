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
#include "fifo.h"
#include <string.h>



#define SEM_KEY 10
#define SHM_KEY 11


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
void sigKill(int sig){
    printf("Kill");
}

int main(int argc, char const *argv[]) {

    char *fifoclient2serverpath = "/home/mario00/Scrivania/Progetto Sistemi Operativi/client2server";
    char *fifoserver2clientpath = "/home/mario00/Scrivania/Progetto Sistemi Operativi/server2client";
    pid_t pidClient = getpid();

    if (argc != 2)
        ErrExit("Sintassi sbagliata\nLa sintassi corretta è: ./client nomegiocatore\n");

    /** INVIO PID GIOCATORE A SERVER **/
    //opening fifo
    int fd = openFifo(fifoclient2serverpath, O_WRONLY);
    sprintf(buffer, "%d", pidClient);
    if (write(fd, buffer, sizeof(buffer)) == -1) {
        ErrExit("error fifoClientToServer read");
    }
    closeFifo(fd);

    /** RICEVO AVVISO DI ATTESA */
    //opening fifo
    int fifoServerToClient = createFifo(fifoserver2clientpath, S_IRUSR | S_IWUSR);
    char riceveMessaggioAttesa[50];
    int fd2 = openFifo(fifoserver2clientpath, O_RDONLY);
    if (read(fd, riceveMessaggioAttesa, sizeof(riceveMessaggioAttesa)) == -1) {
        ErrExit("error first write");
    }
    printf("messaggio attesa: %s\n", riceveMessaggioAttesa);
    closeFifo(fd2);

    /** GIOCATORE 1 RIMANE IN ATTESA FINO A QUANDO NON VIENE TROVATO GIOCATORE 2 */
        char trovatoGiocaotore[100];
        fd2 = openFifo(fifoserver2clientpath, O_RDONLY);
        if (read(fd, trovatoGiocaotore, sizeof(trovatoGiocaotore)) == -1) {
            ErrExit("error first write");
        }
        printf("%s\n", trovatoGiocaotore);
        closeFifo(fd2);



    removeFifo(fifoserver2clientpath);





    
    return 0;
}
