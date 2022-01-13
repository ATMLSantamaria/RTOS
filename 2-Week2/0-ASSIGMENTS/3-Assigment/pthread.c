#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <syslog.h>
#define COUNT  1000
#define N_THREADS 128

typedef struct
{
    int threadIdx;
} threadParams_t;

struct sched_param fifo_sched;


void print_scheduler(void);

//Starter Thread, attribute and function
pthread_t starter_thread;
pthread_attr_t starter_attr;
void * starter_func(void * a);

//working Threads, param and attributes
pthread_t workers_thread[N_THREADS];
threadParams_t workers_param[N_THREADS];
pthread_attr_t workers_attr[N_THREADS];

struct sched_param worker_sched_param[N_THREADS];
void * working_func(void * working_attr);

// Set the attributes to FIFO
void set_scheduler(void){

    // Primero attributos de starter
    //Init
    pthread_attr_init(&starter_attr);
    pthread_attr_setschedpolicy(&starter_attr,SCHED_FIFO); //Set to RT extension policy
    fifo_sched.sched_priority=sched_get_priority_max(SCHED_FIFO);

    sched_setscheduler(getpid(), SCHED_FIFO, &fifo_sched); //give SCHED_FIFO a main tambien

    pthread_attr_setschedparam(&starter_attr,&fifo_sched);
    cpu_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(1,&cpu);
    pthread_attr_setaffinity_np(&starter_attr,sizeof(cpu_set_t),&cpu);

    for (int i =0;i < N_THREADS;i++){

        workers_param[i].threadIdx=i;
        pthread_attr_init(&workers_attr[i]);
        pthread_attr_setschedpolicy(&workers_attr[i],SCHED_FIFO);
        worker_sched_param[i].sched_priority=sched_get_priority_max(SCHED_FIFO)-i;
        pthread_attr_getschedparam(&workers_attr[i],&worker_sched_param[i]);
        cpu_set_t cpu_thread;
        CPU_ZERO(&cpu_thread);
        CPU_SET(3,&cpu_thread);
        pthread_attr_setaffinity_np(&workers_attr[i],sizeof(cpu_set_t),&cpu_thread);

        pthread_create(&workers_thread[i],
                        &workers_attr[i],
                        working_func,
                        (void *)&workers_param[i]);

    }

    
}





int main(){

    set_scheduler();
    

    pthread_create(&starter_thread,
                    &starter_attr,
                    starter_func,
                    (void *)0);

    
    pthread_join(starter_thread,NULL);

}

void * starter_func(void * arg){
    
    struct sched_param res;
    int pid = getpid();
    int rc=sched_getparam(pid, &res);
    int b = res.sched_priority;
    int algo = sched_get_priority_max(SCHED_FIFO);
    //int a = getpriority(gettid(),0);
    //printf("Hola desde Starter con prioridad %d \n",b);
    //printf("MAxima prioridad es = %d\n",algo);


   // print_scheduler();
}

void * working_func(void * working_attr){
    threadParams_t * tmp = (threadParams_t *)working_attr;
    int sum=0;
    for (int i=0;i <= tmp->threadIdx;i++){
        sum+=i;
    }
    int ncpu=sched_getcpu();
    //printf("[COURSE:1][ASSIGNMENT:3]: Thread idx=%d, sum[0...%d]=%d Running on core : %d\n",tmp->threadIdx, tmp->threadIdx, sum,ncpu);
    syslog(LOG_CRIT,"[COURSE:1][ASSIGNMENT:3]: Thread idx=%d, sum[0...%d]=%d Running on core : %d\n",tmp->threadIdx, tmp->threadIdx, sum,ncpu);

    //print_scheduler();
}

void print_scheduler(void)
{
    int schedType = sched_getscheduler(getpid());

    switch(schedType)
    {
        case SCHED_FIFO:
            printf("Pthread policy is SCHED_FIFO\n");
            break;
        case SCHED_OTHER:
            printf("Pthread policy is SCHED_OTHER\n");
            break;
        case SCHED_RR:
            printf("Pthread policy is SCHED_RR\n");
            break;
        default:
            printf("Pthread policy is UNKNOWN\n");
    }
}