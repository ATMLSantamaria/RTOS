#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <syslog.h>
#define N_THREADS 128

typedef struct
{
    int threadIdx;
} threadParams_t;



void print_scheduler(void);

//Starter Thread, attribute and function
pthread_t starter_thread;
pthread_attr_t starter_attr;
struct sched_param fifo_sched;

void * starter_func(void * a);

//working Threads, param and attributes
pthread_t workers_thread[N_THREADS];
threadParams_t workers_param[N_THREADS];
pthread_attr_t workers_attr[N_THREADS];

struct sched_param worker_sched_param[N_THREADS];
void * working_func(void * working_attr);

// Set the attributes to FIFO
void set_scheduler(void){

    // First set starter thread policy to FIFO and affinity. It will run on the core 1
    pthread_attr_init(&starter_attr);
    pthread_attr_setschedpolicy(&starter_attr,SCHED_FIFO); //Set to RT extension policy
    fifo_sched.sched_priority=sched_get_priority_max(SCHED_FIFO);

    sched_setscheduler(getpid(), SCHED_FIFO, &fifo_sched); //give SCHED_FIFO a main

    pthread_attr_setschedparam(&starter_attr,&fifo_sched);
    cpu_set_t cpu;
    CPU_ZERO(&cpu);
    CPU_SET(1,&cpu);
    pthread_attr_setaffinity_np(&starter_attr,sizeof(cpu_set_t),&cpu);

    // Now set working threads policy to FIFO and priorities so the less index max priority. So logs will be ordererd
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
    }

    
}


int main(){


    // Invoke the function to set the policy
    set_scheduler();
    
    // Create the starter FIFO thread that will launch others
    pthread_create(&starter_thread,
                    &starter_attr,
                    starter_func,
                    (void *)0);

    // Join the starter thread
    pthread_join(starter_thread,NULL);

}

void * starter_func(void * arg){
    
    struct sched_param res;
    int pid = getpid();
    int rc=sched_getparam(pid, &res);
    int b = res.sched_priority;

    for (unsigned int i = 0;i<N_THREADS;i++){

        pthread_create(&workers_thread[i],
                        &workers_attr[i],
                        working_func,
                        (void *)&workers_param[i]);
    }




    for(unsigned int i = 0;i<N_THREADS;i++){
        pthread_join(workers_thread[i],NULL);
    }
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