/* 
 * Dos procesos sincronizados muy basico
 *
 * by Sam Siewert
 *
 */
#include	<stdio.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/wait.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/stat.h>
#include	<semaphore.h>

#define TRUE (1)
#define FALSE (0)

int main(){

    // PID del chil process
    int chPID; 
    pid_t thisChPID;
    int stat;
    
    // Semaphores Name
    char syncSemChildName[]="/twoprocChildsync";
    char syncSemParentName[]="/twoprocParentsync"; //Formato /

    char syncSemCName[]="/twoprocCsync";
    char syncSemPName[]="/twoprocPsync";


    // Semaphores 
    sem_t *syncSemChild,*syncSemParent;
    // open Semaphores
    syncSemChild = sem_open(syncSemChildName,O_CREAT,0700,0);
                            //O_CREAT, //create new semaphore, alternative O_EXCL
                            //0700, //mode (input) Permission flags. 
                           //0);   //value
    printf("twprocs syncSemChild done\n");
    
    syncSemParent = sem_open(syncSemParentName,O_CREAT,0700,0);
                           // O_CREAT,
                            //0700, //(input) Permission flags. 
                           // 0);*/
    printf("twprocs syncSemParent done\n");

    if((chPID = fork()) == 0){ //this is the child
        long int i=0;
        
        printf("CHILD process\n");

        while(i<5){
            printf("i = %ld\n",i);
            i++;
        }
        sem_post(syncSemParent);
        printf("Voy a despertar a PARENT\n");
        sem_close(syncSemParent);
        exit(0);
    } else {
        printf("PARENT process\n");
        printf("Voy a dormir a PARENT\n");
        sem_wait(syncSemParent); /* DETIENE A CHILD? */

        printf("Parent desperto\n");
        long int j=0;
        while(j<5){
            printf("jjjj = %ld\n",j);
            j++;
        }
        thisChPID = wait(&stat);
        sem_close(syncSemChild);

        exit(0);
    }
}