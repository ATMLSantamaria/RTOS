#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <unistd.h>

#define NUM_THREADS 64
#define NUM_CPUS 8

typedef struct {
  int threadIdx;
} threadParams_t;


// POSIX declarations and scheduling attributes
// POSIX declaration
pthread_t myThreads[NUM_THREADS];
pthread_t myMainThread;
pthread_t myStartThread;
threadParams_t threadParams[NUM_THREADS];

// Scheduling attributes
pthread_attr_t fifo_sched_attr;
pthread_attr_t orig_sched_attr;
struct sched_param fifo_param;



#define SCHED_POLICY SCHED_FIFO
#define MAX_ITERATIONS (1000000)


void print_scheduler(void) {
  //Use function sched_getscheduler in the process you are
  int schedType = sched_getscheduler(getpid());

  switch(schedType){
        case SCHED_FIFO:
            printf("Pthread policy is SCHED_FIFO\n"); //The one we will generally use
            break;
        case SCHED_OTHER:
            printf("Pthread policy is SCHED_OTHER\n"); // This policy is default in RedHat8
            break;
        case SCHED_RR:
            printf("Pthread policy is SCHED_RR\n"); // This policy use round Robin algorithm
            break;
        default:
            printf("Pthread policy is UNKNOWN\n");
  }  
}

void set_scheduler(void) {
  int max_prio,scope,rc,cpuidx;
  cpu_set_t cpuset;
  
  printf("INITIAL ");
  print_scheduler();

  pthread_attr_init(&fifo_sched_attr); //initialise the pthread_attr_t type
  pthread_attr_setinheritsched(&fifo_sched_attr,PTHREAD_EXPLICIT_SCHED); //set the inheritance
  pthread_attr_setschedpolicy(&fifo_sched_attr,SCHED_FIFO); //set the FIFO policy
  CPU_ZERO(&cpuset); // initialise the set to  be empty

  cpuidx = 5;
  CPU_SET(cpuidx,&cpuset);
  pthread_attr_setaffinity_np(&fifo_sched_attr,sizeof(cpu_set_t),&cpuset); //set the scheduling policy in the cpu 5

  max_prio = sched_get_priority_max(SCHED_FIFO); //get max prio in this system for this policy
  fifo_param.sched_priority = max_prio; //put it into the fifo_param

  if((rc=sched_setscheduler(getpid(), SCHED_POLICY, &fifo_param)) < 0) {
        perror("sched_setscheduler");
  }
  pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);

  printf("ADJUSTED "); print_scheduler();
  
}

//The function pthread_create imposses that the start toutine return void * and accept
// (void *)
void * counterThread(void * threadp) {
  int sum = 0, i, rc, iterations;
  threadParams_t * threadParams = (threadParams_t *)threadp;
  
  pthread_t myThread;
  double start = 0.0,stop = 0.0;
  struct timeval startTime, stopTime;

  gettimeofday(&startTime,0);
  
  //tv_sec are the second and tv_usec the microsecons
  start = ((startTime.tv_sec * 1000000.0) + startTime.tv_usec)/1000000.0;

  for (iterations = 0;iterations < MAX_ITERATIONS;++iterations) {

    sum = 0;
    for (i = 1; i <= threadParams->threadIdx;++i) {
      sum += i;
    }
  }
  
  gettimeofday(&stopTime,0);
  
  stop = ((stopTime.tv_sec * 1000000.0) + stopTime.tv_usec)/1000000.0;

  
  printf("\nThread idx=%d, sum[0...%d]=%d, running on CPU=%d, start=%lf, stop=%lf", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum, sched_getcpu(),
           start, stop);
}

void * starterThread(void * threadp) {
  int i,rc;

  printf("starter thread running on CPU = %d\n",sched_getcpu());

  for (i = 0; i < NUM_THREADS;++i) {
    threadParams[i].threadIdx = i;
     
    pthread_create(&myThreads[i],   // pointer to the thread descriptor
                   &fifo_sched_attr, // use FIFO RT MAX priotiry attributes
                   counterThread,   // thread function entry point
                   (void *)&(threadParams[i])
                   );
  }

 
  for(i=0;i<NUM_THREADS;i++) {
    pthread_join(myThreads[i], NULL);
  }
}


int main(int argc, char *argv[]) {
  int rc;
  int i,j;
  cpu_set_t cpuset;

  set_scheduler();

  CPU_ZERO(&cpuset);

  // get affinity set for main thread
  myMainThread = pthread_self();

  // Check the affinity mask assigned to the thread

  rc = pthread_getaffinity_np(myMainThread,sizeof(cpu_set_t),&cpuset);

  if (rc != 0 ) {
    perror("pthread_get_Affinitynp");
  } else {
    printf("main thread running on CPU = %d, CPUs =",sched_getcpu());

    for (j = 0; j < CPU_SETSIZE; j++) {
           if (CPU_ISSET(j, &cpuset)) {
             printf(" %d", j);
           }
    }
    printf("\n");
  }
  pthread_create(&myStartThread,   // pointer to thread descriptor
                  &fifo_sched_attr,     // use FIFO RT max priority attributes
                  starterThread, // thread function entry point
                  (void *)0 // parameters to pass in
                 );

   pthread_join(myStartThread, NULL);

   printf("\nTEST COMPLETE\n");
}
