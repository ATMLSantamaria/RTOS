#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#define NUMBER_OF_THREADS 64

typedef struct {
  int threadIdx;
} threadParams_t;

//POSIX Thread declaration

pthread_t myThreads[NUMBER_OF_THREADS];
threadParams_t myThreadsParams[NUMBER_OF_THREADS];

// Function  to initiate thread

void * counterThread(void * threadp) {
  
  int i = 1;
  int sum = 0;

  threadParams_t * localThreadParam = (threadParams_t *)threadp;

  for (i; i <= (localThreadParam->threadIdx);++i) {
    sum += i;
  }

  printf("Thread idx = %d, sum[1...%d] = %d\n",
        localThreadParam->threadIdx,
        localThreadParam->threadIdx, sum);
}


int main (int argc, char * argv[]) {
  int rc = 0;
  int i = 1;

  for (i; i <= NUMBER_OF_THREADS; ++i) {

    myThreadsParams[i].threadIdx = i;
    
    pthread_create(&myThreads[i], //pointer to thread descriptor
                   (void *)0,   //default attributes
                   counterThread, //thread function entrypoint
                   (void *)&(myThreadsParams[i])  //parameters to pass in
                   );
  }

  for (i = 0; i < NUMBER_OF_THREADS;++i) {

    pthread_join(myThreads[i],NULL);

  }
  printf("TEST COMPLETE\n");
}
