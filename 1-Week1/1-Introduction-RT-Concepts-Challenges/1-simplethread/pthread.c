#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/utsname.h>
#define NUM_THREADS 12

typedef struct
{
    int threadIdx;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];


void *counterThread(void *threadp)
{
    int sum=0, i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=1; i < (threadParams->threadIdx)+1; i++)
        sum=sum+i;
 
    printf("Thread idx=%d, sum[0...%d]=%d\n", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum);
    syslog(LOG_CRIT,"[COURSE:1][ASSIGNMENT:1]:Hello World from Thread!");

}

int main (int argc, char *argv[])
{
   int rc;
   int i;
    syslog(LOG_CRIT,"[COURSE:1][ASSIGNMENT:1]:Hello World from Main!");

   for(i=0; i < NUM_THREADS; i++)
   {
       threadParams[i].threadIdx=i;

       pthread_create(&threads[i],   // pointer to thread descriptor
                      (void *)0,     // use default attributes
                      counterThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );

   }

   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

    printf("TEST COMPLETE\n");
    struct timeval tv;
    struct utsname unameData;
    uname(&unameData);
    //He usado uname -a en el terminal finalmente
    //printf(/*"[COURSE:1][ASSIGNMENT:1]:*/"%s %s %s %s %s\n",unameData.sysname,unameData.nodename,unameData.version,unameData.machine,unameData.release);
    //syslog(LOG_CRIT,"[COURSE:1][ASSIGNMENT:1]:Hello World from Main!");
    //syslog(LOG_CRIT, "My log message test @ tv.tv_sec %ld, tv.tv_usec %ld\n", tv.tv_sec, tv.tv_usec);
}
