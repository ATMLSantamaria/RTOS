#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
//FIFO Scheduling se usa para RT extensions in Linux
//Estructura usada para pasar lo que sea necesario a un thread
typedef struct 
{
    int threadIdx;
    double salario;
    char pais[];
}thread_prametros_t;

#define NUM_THREADS 64
#define NUM_CPUS 8

//POSIX declarations needed for threads
// Tres declaraciones de threads

pthread_t threads[NUM_THREADS];
pthread_t mainThread;
pthread_t startThread;
thread_prametros_t threadParams[NUM_THREADS];

// Atributos para crea los threads
pthread_attr_t fifo_sched_attr; //este lo voy a usar para los fifo threads
pthread_attr_t orig_schd_attr;
struct sched_param fifo_param;

#define SCHED_POLICY SCHED_FIFO

/*
    FUNCTION PARA IMPRIMIR LA SCHED POLICY DE CADA THREAD
*/
void print_scheduler(void){
    int a = getpid();
    int schedType = sched_getscheduler(getpid());
    //getpid() return the process id of calling process
    switch(schedType){
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

void set_scheduler(void){
    //Tipo usado para colocar el proceso donde queramos
    cpu_set_t cpuset;

    //Definimos la variable attributos con las funciones necesarias
    //Inicializamos
    pthread_attr_init(&fifo_sched_attr);
    //Definimos herencia-> PTHREAD_EXPLICIT_SCHED significa no herencia, debemos definir attributos de scheudling nosotros
    pthread_attr_setinheritsched(&fifo_sched_attr,PTHREAD_EXPLICIT_SCHED);
    //Por ello definimos el scheduling a continacion -> Definimos fifo
    pthread_attr_setschedpolicy(&fifo_sched_attr,SCHED_POLICY);

    //Primero siempre hay que hacer 0 el parametro cpu set
    CPU_ZERO(&cpuset);
    int cpuidx =(5);
    //Con CPU_SET fijamos en que CPU vamos a correr
    CPU_SET(cpuidx,&cpuset);
    //Con la siguiente funcion, fijamos en el attributo el valor de la CPU en la que el thread creado con ese atributo va a correr
    pthread_attr_setaffinity_np(&fifo_sched_attr,sizeof(cpu_set_t),&cpuset);

    //Ahora fijamos la prioridad a la maxima posible dentro de la politica FIFO
    //El rango es 0-99
    int max_prio=0;
    max_prio=sched_get_priority_max(SCHED_POLICY);
    printf("Max priority=%d\n",max_prio);
    //Anadimos a los parametros
    fifo_param.sched_priority=max_prio;

    //Ahora hay que fijar la politica en el proceso y en los attr que vamos a pasar al thread
    //La funcion debajo fija la Policy y los paramteros (priority) del thread especificado, en este caso del thread que incova la funcion
    //OJO PARA AFFINITY NECESITAS SER SUDO
    sched_setscheduler(getpid(),SCHED_POLICY,&fifo_param); //SI EJECUTAS SIN SUDO NO FUNCIONA
    //printf("ADJUSTED ");
    //print_scheduler();

    //Ahora fijamos la priority en el attributo para crear threads
    pthread_attr_setschedparam(&fifo_sched_attr,&fifo_param);

}   

void *counterThread(void *threadp)
{
    thread_prametros_t * a = (thread_prametros_t *)threadp;
    printf("Hola\n");
    printf("Mi salario es =%f\n",a->salario);
}

//Funcion para crear un thread
void * primer_thread(void *thread_primero){
    printf("Primer thread creado, running on CPU=%d\n",sched_getcpu());
    //thread_prametros_t 
    threadParams[0].salario=2;
    threadParams[1].salario=200;
    threadParams[2].salario=20000;

    threadParams[1].pais[0]='E';
    threadParams[2].pais[0]='A';
    //Desde aqui creo mas threads
    for(int i=0; i < 3; i++){
       threadParams[i].threadIdx=i;
    
       pthread_create(&threads[i],   // pointer to thread descriptor
                      &fifo_sched_attr,     // use FIFO RT max priority attributes
                      counterThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );

    }
    for (int i=0;i<3;i++){
        pthread_join(threads[i],NULL);
    }
}



int main(){
    cpu_set_t cpuset;int i,j;
    print_scheduler();
    //set_scheduler();
    //print_scheduler();

    //Primero analizamos el MAIN THREAD
    mainThread = pthread_self();

    //Miramos donde esta corriendo el MAIN THREAD
    pthread_getaffinity_np(mainThread,sizeof(cpu_set_t),&cpuset);

    printf("MAIN THREAD running on CPU=%d\nCPUs disponibles: ",sched_getcpu()); 
    //si ejecutamos varias veces vemos que la CPU donde se ejecuta es aleatoria
    for (j = 0; j < CPU_SETSIZE; j++)
           if (CPU_ISSET(j, &cpuset))
               printf(" %d", j);
    printf("\n");

    //Llamo a la funcion que he creado para ajustar affinity y scheduling
    set_scheduler();
    printf("AHORA MAIN THREAD running on CPU=%d\nCPUs disponibles: ",sched_getcpu()); 

    //Creo el primer Thread con los attributos de scheduling y priority
    pthread_create(&startThread,&fifo_sched_attr,primer_thread,(void *)0);


    //Join the thread
    pthread_join(startThread,NULL);
}
