#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/utsname.h>

//#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#define NUM_THREADS 12

typedef struct
{
    int threadIdx;
} threadParams_t;

pthread_t my_posix_thread;
threadParams_t threadParams;

void *imprimir_en_thread(void *threadp){
    printf("Hello desde el POSIX thread\n Tu edad es %d",29);
}

int main(int arcv,char *argv[]){
    int edad = 29;

    fork(); //este fork divide en dos procesos iguales todo el codigo aguas abajo

    pthread_create(&my_posix_thread,
                    (void *)0,
                    imprimir_en_thread,
                    (void *)&threadParams);

    pthread_join(my_posix_thread,NULL);
    printf("hola\n");
    return 0;
}