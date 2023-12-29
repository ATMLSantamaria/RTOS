#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#define COUNT 1000

typedef struct 
{
  int threadIdx;
} threadParams_t;


// Define a POSIX thread declaration
pthread_t myThreads[2];
threadParams_t myThreadsParams[2];

// Global variable (that is unsafe)
int gsum = 0;

// Function to increment thread
void *incThread (void *threadp) {
  int i = 0;

  threadParams_t * threadParams = (threadParams_t *)threadp;

  for (i; i < COUNT; ++i) {
    
    gsum += i;
    printf("Increment thread idx=%d,gsum=%d\n",threadParams->threadIdx,gsum);

  }

}

// Function to decrement thread
void *decThread(void *threadp) {
  int i = 0;

  threadParams_t * threadParams = (threadParams_t *)threadp;

  for (i; i < COUNT; ++i) {
    
    gsum -= i;
    printf("Decrement thread idx=%d,gsum=%d\n",threadParams->threadIdx,gsum);

  }
}

// As we can see both threads will simultaneusly try to modify one global variable without any protection


int main (int argc,char *argv[]) {
  

  int rc;
  int i = 0;


  myThreadsParams[i].threadIdx = i;
  
  //Use function from thread.h to create a thread
  pthread_create(&myThreads[i], //pointer to the thread descriptor (name)
                 (void *)0,     //use default attributes 
                 incThread,     //thread function entrypoint
                 (void *)&(myThreadsParams[i]) //paramters to pass in
                 );

  ++i;
  // Create a second thread
  myThreadsParams[i].threadIdx = i;
  
  pthread_create(&myThreads[i],
                 (void *)0,
                 decThread,
                 (void *)&(myThreadsParams[i])
                 );
  



  //Join the thread using pthread_join function

  pthread_join(myThreads[0],NULL);
  pthread_join(myThreads[1],NULL);

}






