#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>

#define COUNT  1000
#define NUM_T 128

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_T];
threadParams_t threadParams[NUM_T];

// It is not necessary for assignment but for out clarity I prefer a FIFO scheduling
pthread_attr_t fifo_sched_attr;

// Unsafe global
int gsum=0;

void *incThread(void *threadp)
{

    // To the thread a VOID pointer is passed. We need to conciusly cast this pointer to the Type of data we pass from main to the thread.
    threadParams_t *threadParams = (threadParams_t *)threadp;

    // We Increase the global variable.
    gsum=gsum+threadParams->threadIdx;
    // Summation variable
    int sum = 0;

    for (int i=1;i<=threadParams->threadIdx;i++){
        sum+=i;
    }

    printf("[COURSE:1][ASSIGNMENT:2]: Thread idx=%d,sum[0...%d]=%d\n",threadParams->threadIdx, threadParams->threadIdx, sum);
    syslog(LOG_CRIT,"[COURSE:1][ASSIGNMENT:2]: Thread idx=%d, sum[0...%d]=%d\n",threadParams->threadIdx, threadParams->threadIdx, sum);
}

/*
void *decThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        gsum=gsum-i;
        printf("Decrement thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
}
*/



int main (int argc, char *argv[])
{
   int rc;
   int i=0;


    //initialize attributes for thread
    pthread_attr_init(&fifo_sched_attr);
    //Set policy to FIFO
    pthread_attr_setschedpolicy(&fifo_sched_attr,SCHED_FIFO);

   // Loop to assign Idx to each thread and create each thread.
   // NUM_T is a macro to define the numbers of threads we want to create

   for(i=0;i<NUM_T;i++){ 
        threadParams[i].threadIdx=i;
        pthread_create(&threads[i],   // pointer to thread descriptor
                        //(void *)0,     // use default attributes
                        &fifo_sched_attr,
                        incThread, // thread function entry point
                        (void *)&(threadParams[i]) // parameters to pass in
                        );
   }

    // Loop to join each thread
   for(i=0; i<NUM_T; i++)
     pthread_join(threads[i], NULL); //Since we dont need any pointer coming back from thread second argument is NULL


   printf("TEST COMPLETE\n");
}
