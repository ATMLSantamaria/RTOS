#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_THREADS (8)
#define NUM_CPUS (8)

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_MSEC (1000000)
#define NSEC_PER_MICROSEC (1000)
#define DELAY_TICKS (1)
#define ERROR (-1)
#define OK (0)

unsigned int idx = 0;
unsigned int jdx = 1;
unsigned int seqIterations = 47;
unsigned int reqIterations = 100000;

// Volatile indicate the compiler that this variable can be changed from outside
// By concurrent operations, so dont assume its value is unchanged
volatile unsigned int fib = 0;
volatile unsigned int fib0 = 0;
volatile unsigned int fib1 = 1;

int FIB_TEST (unsigned int seqCnt, unsigned int iterCnt) {
  for (idx = 0; idx < iterCnt; ++idx) {
    fib = fib0 + fib1;
    while (jdx < seqCnt) {
      fib0 = fib1;
      fib1 = fib;
      fib = fib0 + fib1;
      ++jdx;
    }
    jdx = 0;
  }
  return idx;
}

typedef struct {
  int threadIdx;
} threadParams_t;


//POSIX THREAD DECLARATION
// Declaration of theads
pthread_t myThreads[NUM_THREADS];
// Declare attributes of threads -> This is use to create threads with aprticular sched policy
pthread_attr_t rt_myThread_attr[NUM_THREADS];
pthread_attr_t main_attr;
// Declare the parametes of threads
threadParams_t myThreadParams[NUM_THREADS];
// Int for priorities
int rt_max_prio, rt_min_prio;
// Declare params for scheduling -> This is used to set sched policy in existing thread
struct sched_param rt_myThread_params[NUM_THREADS];
struct sched_param mainThread_param;
// process id on the main process
pid_t mainpid;

//Function to compute delta in time
int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t);

//Function to print the sched policy in the invoking thread

void print_scheduler(void);

#define SUM_ITERATIONS (1000)

//Counter thread. Function pointer to launch in each rt_thread, make some computation and count time
void * counterThread(void * threadp) {
  int sum = 0,i;
  
  struct timespec start_time = {0,0};
  struct timespec finish_time = {0,0};
  struct timespec thread_dt = {0,0};

  threadParams_t * thisThreadParam = (threadParams_t *)threadp;

  //Get start time
  clock_gettime(CLOCK_REALTIME,&start_time);

  //Compute something to spend time
  
  // COMPUTE SECTION
  for(i=1; i < ((thisThreadParam->threadIdx)+1*SUM_ITERATIONS); i++) {
    sum=sum+i;
  }
  FIB_TEST(seqIterations, reqIterations);
  // END COMPUTE SECTION

  //Get end time
  clock_gettime(CLOCK_REALTIME,&finish_time);
 
  //Compute difference between both times
  delta_t(&finish_time,&start_time,&thread_dt);


  printf("\nThread idx=%d ran %ld sec, %ld msec (%ld microsec, %ld nsec) on core=%d\n",
	thisThreadParam->threadIdx, thread_dt.tv_sec, (thread_dt.tv_nsec / NSEC_PER_MSEC),
	(thread_dt.tv_nsec / NSEC_PER_MICROSEC), thread_dt.tv_nsec, sched_getcpu());
  
pthread_exit(&sum);
}

int main(int argc, char *argv[]) {
  
  // Some initial stuff
  int rc, idx;
  
   printf("This system has %d processors with %d available\n", get_nprocs_conf(), get_nprocs());
   printf("The test thread created will be SCHED_FIFO is run with sudo and will be run on least busy core.\nThis is because we dont set affinity to one core, so Linux will do SMP and run in the least busy core\n\n");


  printf("Main thread sched policy before we set it it: ");
  print_scheduler();
  printf("\n");  
  


  printf("Setting sched policy...\n");
  // Set sched policy in main threads
  // 1 - Get pid
  mainpid = getpid();
  // 2 - Get max and min priorities in this systems
  rt_max_prio = sched_get_priority_max(SCHED_FIFO);
  rt_min_prio = sched_get_priority_min(SCHED_FIFO);
  // 3 - Get the reference to param to apply it in this process/thread
  rc = sched_getparam(mainpid,&mainThread_param);
  // 4 - Set in that param the priority
  mainThread_param.sched_priority = rt_max_prio;
  // 5 - Now apply it
  if((rc = sched_setscheduler(getpid(),SCHED_FIFO,&mainThread_param) < 0)) {
    perror("********** WARNING: sched_setscheduler");
    exit(-1);
  }
  printf("... setting policy finished\n");
  
  
  printf("Main thread sched policy AFTER we set it it: ");
  print_scheduler();
  printf("\n");  

  // Create rt_threads with the correct policy
  for (int i = 0; i < NUM_THREADS;++i) {
    // 1 - Initialise attribute
    rc = pthread_attr_init(&rt_myThread_attr[i]);
    // 2 - Set the desired attributei for inheritance
    rc = pthread_attr_setinheritsched(&rt_myThread_attr[i],PTHREAD_EXPLICIT_SCHED);
    // 3 - Set the desired attibute for sched
    rc = pthread_attr_setschedpolicy(&rt_myThread_attr[i],SCHED_FIFO);
    // 4 - Set the desired priority
    rt_myThread_params[i].sched_priority = rt_max_prio - 1;
    pthread_attr_setschedparam(&rt_myThread_attr[i],&rt_myThread_params[i]);
    
    // 5 - Put whatever we want in the parameters we pass to the tread
    myThreadParams[i].threadIdx = i;

    // 6 - Call pthread_create to crate the thread
    pthread_create(&myThreads[i],           //thread descriptor
                   &rt_myThread_attr[i],   //atributes for the thread
                   counterThread,           // thread function entry point
                   (void *)&(myThreadParams[i])
                   );
  }
  

  //Join the rt_threads
  for (int i = 0; i < NUM_THREADS;++i) {
    pthread_join(myThreads[i],NULL);
  }
  
  printf("\nObserve that in each run each thread is running in random cores.\n");
  printf("This is because Linux by default will do SMP, symetric multiprocessing.\n");
  printf("If we dont set affinity Linux simply chooses the least busy core.\nWe can change this and decide in which core the thread should run.\nSwitching to AMP, asymetric multiprocessing.\n");
  printf("TEST COMPLETE\n");
}


void print_scheduler(void)
{
   int schedType, scope;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_OTHER\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }

   pthread_attr_getscope(&main_attr, &scope);

   if(scope == PTHREAD_SCOPE_SYSTEM)
     printf("PTHREAD SCOPE SYSTEM\n");
   else if (scope == PTHREAD_SCOPE_PROCESS)
     printf("PTHREAD SCOPE PROCESS\n");
   else
     printf("PTHREAD SCOPE UNKNOWN\n");

}





int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  //printf("\ndt calcuation\n");

  // case 1 - less than a second of change
  if(dt_sec == 0)
  {
	  //printf("dt less than 1 second\n");

	  if(dt_nsec >= 0 && dt_nsec < NSEC_PER_SEC)
	  {
	          //printf("nanosec greater at stop than start\n");
		  delta_t->tv_sec = 0;
		  delta_t->tv_nsec = dt_nsec;
	  }

	  else if(dt_nsec > NSEC_PER_SEC)
	  {
	          //printf("nanosec overflow\n");
		  delta_t->tv_sec = 1;
		  delta_t->tv_nsec = dt_nsec-NSEC_PER_SEC;
	  }

	  else // dt_nsec < 0 means stop is earlier than start
	  {
	         printf("stop is earlier than start\n");
		 return(ERROR);  
	  }
  }

  // case 2 - more than a second of change, check for roll-over
  else if(dt_sec > 0)
  {
	  //printf("dt more than 1 second\n");

	  if(dt_nsec >= 0 && dt_nsec < NSEC_PER_SEC)
	  {
	          //printf("nanosec greater at stop than start\n");
		  delta_t->tv_sec = dt_sec;
		  delta_t->tv_nsec = dt_nsec;
	  }

	  else if(dt_nsec > NSEC_PER_SEC)
	  {
	          //printf("nanosec overflow\n");
		  delta_t->tv_sec = delta_t->tv_sec + 1;
		  delta_t->tv_nsec = dt_nsec-NSEC_PER_SEC;
	  }

	  else // dt_nsec < 0 means roll over
	  {
	          //printf("nanosec roll over\n");
		  delta_t->tv_sec = dt_sec-1;
		  delta_t->tv_nsec = NSEC_PER_SEC + dt_nsec;
	  }
  }

  return(OK);
}

