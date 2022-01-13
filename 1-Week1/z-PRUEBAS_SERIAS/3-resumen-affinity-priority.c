/*
    Afinnity - Decidimos o no en que core corre el thread 
                - Si no decidimos - Linux por defecto es Symetic Multiprocessing con load balance. 
                  Va a mover la carga computaiconal entre cores segun le parezca. 
                - Si decidimos - CPU_ZERO y CPU_SET (n) y decimos el core o cores donde corre la application 
    Priority - DEcidimos o no la prioridad:
                -No decidimos - Por defecto Linux es completily fair priority. No hay ninguna tarea con pripridad y las va ejecutando a la vez, activandolas
                y desactivandolas segun van progresando. policy SCHED_OTHER, que es la que hay por defecto
                - Decidimos - Fixed priority - En este caso nosotros decidimos la prioridad, las tareas se ejectuan com preemtive run to completion. La tarea
                con maxima prioridad interrumpe a cualquier otra y corre hasta que se termina. Por defecto todas las tareas en este modo tienen prioridad estatica
                mayor que 1, por lo que siempre interrumpen a cualquier tarea con policy SCHED_OTHER. 
                Esta politica se impone con SCHED_FIFO
*/

#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

#define COUNT 1000
#define NUM_THREADS 64
#define NUM_CPUS 8
#define MAX_ITERATIONS (1000000)

typedef struct
{
    int threadIdx;
} threadParams_t;

// Unsafe global for give work to threads
int gsum=0;

//Funciones para alargar la computacion y estudiarla en htop
void *incThread(void *threadp);
void *decThread(void *threadp);
void *starterThread(void *threadp);

int main(){

    // SIN AFINIDAD NI PRIORIDAD;
    pthread_t thread_sin_affinity_ni_priority[2];
    threadParams_t param_sin_affinity_ni_priority[2];
    param_sin_affinity_ni_priority[0].threadIdx=0;
    param_sin_affinity_ni_priority[1].threadIdx=1;

    // Threads con affinidad y priority
    pthread_t creator;
    pthread_attr_t attr_creator;
    cpu_set_t cpu_creator;
    struct sched_param schedul_param_creator;
    threadParams_t param_creator;
    param_creator.threadIdx=1000;

    //Inicializar attributos
    pthread_attr_init(&attr_creator);
    // Establezco que la politica del thread es FIFO
    pthread_attr_setschedpolicy(&attr_creator,SCHED_FIFO);

    // Establezco AFFINITy, donde va a correr el thread
    CPU_ZERO(&cpu_creator); //Siempre necesario primero establecer a 0
    CPU_SET(6,&cpu_creator); // tras establecer a 0 podemos establecer el cpu que quedamos, o dejarlo en el cpu 0 que corresponde al 1 en htop
    pthread_attr_setaffinity_np(&attr_creator,sizeof(cpu_set_t),&cpu_creator); //include in the affinity

    // Establezco priority, para ello es necesario el sched_param struct. Por algun motivo es una structura auxiliar que la libreria usa. 
    // La prioridad puede ser cualqueira de 0 a 99 en linux
    // Set params en sched_param
    schedul_param_creator.sched_priority=sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(getpid(),SCHED_FIFO,&schedul_param_creator);
    // set sched_param to attribute
    pthread_attr_setschedparam(&attr_creator,&schedul_param_creator);

    int i = 1;
    //int = 1;
    //int =2;
    if (i == 0){
        pthread_create(&thread_sin_affinity_ni_priority[0],
                        (void *)0,  //deffault attributes
                        incThread,
                        (void *)&(param_sin_affinity_ni_priority[0])   // data have to be passed ass a (void *) 
                        );

        pthread_create(&thread_sin_affinity_ni_priority[1],
                    (void *)0,  //deffault attributes
                    decThread,
                    (void *)&(param_sin_affinity_ni_priority[1])   // data have to be passed ass a (void *) 
                    );

        pthread_join(thread_sin_affinity_ni_priority[0],NULL);
        pthread_join(thread_sin_affinity_ni_priority[1],NULL);
    } else {
        pthread_create(&creator,
                        &attr_creator,
                        starterThread,
                        (void *)&param_creator
                        );
    }
    pthread_join(creator,NULL);
}



void *incThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        gsum=gsum+i;
        printf("Increment thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
    }
}


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








void *counterThread(void *threadp)
{
    int sum=0, i, rc, iterations;
    threadParams_t *threadParams = (threadParams_t *)threadp;
    pthread_t mythread;
    double start=0.0, stop=0.0;
    struct timeval startTime, stopTime;

    gettimeofday(&startTime, 0);
    start = ((startTime.tv_sec * 1000000.0) + startTime.tv_usec)/1000000.0;


    for(iterations=0; iterations < MAX_ITERATIONS; iterations++)
    {
        sum=0;
        for(i=1; i < (threadParams->threadIdx)+1; i++)
            sum=sum+i;
    }


    gettimeofday(&stopTime, 0);
    stop = ((stopTime.tv_sec * 1000000.0) + stopTime.tv_usec)/1000000.0;

    printf("\nThread idx=%d, sum[0...%d]=%d, running on CPU=%d, start=%lf, stop=%lf", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum, sched_getcpu(),
           start, stop);
}

void *starterThread(void *threadp)
{
   int i, rc;

   printf("starter thread running on CPU=%d\n", sched_getcpu());

   for(i=0; i < NUM_THREADS; i++)
   {
    //   threadParams[i].threadIdx=i;

    //    pthread_create(&threads[i],   // pointer to thread descriptor
    //                   &fifo_sched_attr,     // use FIFO RT max priority attributes
    //                   counterThread, // thread function entry point
    //                   (void *)&(threadParams[i]) // parameters to pass in
    //                  );

   }

//    for(i=0;i<NUM_THREADS;i++)
//        pthread_join(threads[i], NULL);

    while(1){
        printf("starter thread running on CPU=%d\n", sched_getcpu());
    }

}
