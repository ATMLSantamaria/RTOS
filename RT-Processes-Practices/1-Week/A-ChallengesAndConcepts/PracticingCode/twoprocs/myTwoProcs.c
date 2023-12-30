/* 
 * Example of 2 processes in a sync relation
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <semaphore.h>



#define TRUE (1)
#define FALSE (0)


int main() {

  int chPID;     // child Process ID
  int stat;      // Used by parent wait
  int rc;        // return code
  int i = 0;
  
  pid_t thisChPID;
  sem_t *syncSemC;   // child semaphore
  sem_t *syncSemP;   // parent semaphore
  
  char syncSemCName[] = "/twoprocCsync";
  char syncSemPName[] = "/twoprocPsync";

  printf("twoprocs\n");

  // Set up syncrhonizing semaphore before fork

  syncSemC =                      // variable to hold the semaphore
           sem_open(syncSemCName, // name of the semaphore
                      O_CREAT,    // flag that indicates that semaphore should be created  
                      0700,       // file permission mode of semaphore. similar to chmod 
                      0           // Initial value of semaphore. 
                                  // 0 indicate to function sem_wait to wait until the value of semaphore is increased
                      );
  printf("twoprocs syncSemC done\n");

  syncSemP = sem_open(syncSemPName,O_CREAT,0700,0);
  printf("twoprocs syncSemP done\n");

  printf("twprocs semaphores set up, caling fork\n");

  if ((chPID = fork()) == 0) { //This is code will be executed in the CHILD
    while(i < 3) {
      printf("Child:   Taking syncC semaphore\n");
      
      if (sem_wait(syncSemC) < 0) perror("sem_wait syncSemC Child");
      
      printf("Child: got syncC semaphore\n");
      
      printf("Child: posting syncP Parent semaphore\n");
      
      if (sem_post(syncSemP) < 0) perror("sem_post syncSemP Child");
      
      //sem_post is used to unlock a semaphore. It increments the semaphore value
      // if other process or thread is blocked in a sem_wait in that semaphore 
      // it will be waken up
      
      printf("Child: posted syncP Parent semaphore\n");

      ++i;
    }
  
  printf("Child is closing down\n");
  if (sem_close(syncSemP) < 0) perror("sem_close syncSemP Child");
  if (sem_close(syncSemC) < 0) perror("sem_close syncSemC Child");

  // sem_close closes the semaphore, and threfore realease any resource associated with it

  exit(0);
  } 
  
  else {
  //  This is the parent
  while ( i < 3) {
    printf("Parent: posting syncC semaphore\n");
    if (sem_post(syncSemC) < 0) perror("sem_post syncSemC Parent");
    
    // sem_post unlock the semaphore on child
    printf("Parent: Posted syncC semaphore\n");

    printf("Parent: taking syncP semaphore\n");
    if (sem_wait(syncSemP) < 0) perror("sem_wait syncSemP Parent");
    printf("Parent: got syncP semaphore\n");

    ++i;    
  }
  
  // Now wait for the child to terminate
  printf("Parent waiting on chuld to termiante\n");
  thisChPID = wait(&stat);

  printf("Parent is closing down\n");
  if (sem_close(syncSemC) < 0) perror("sem_close syncSemC Parent");
  if (sem_close(syncSemP) < 0) perror("sem_close syncSemC Parent");

  if (sem_unlink(syncSemCName) < 0) perror("sem_unlink syncSemCNAme");
  if (sem_unlink(syncSemPName) < 0) perror("sem_unlink syncSemPName");

  exit(0);

  }

}
  
