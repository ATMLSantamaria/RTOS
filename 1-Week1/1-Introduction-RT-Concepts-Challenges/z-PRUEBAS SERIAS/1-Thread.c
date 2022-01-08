#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>


typedef struct thread_data {
   int a;
   int b;
   int result;

} thread_data;

void *myThread(void *arg)
{
   thread_data *tdata=(thread_data *)arg;

   int a=tdata->a;
   int b=tdata->b;
   int result=a+b;

   tdata->result=result;
   thread_data * arg_a_traves_exit = (thread_data *)malloc(sizeof(arg_a_traves_exit));
   arg_a_traves_exit->a=tdata->a;
   arg_a_traves_exit->b=tdata->b;
   arg_a_traves_exit->result=tdata->result;
   //pthread_exit(NULL);
   pthread_exit(arg_a_traves_exit);
}

int main()
{
   pthread_t tid;
   thread_data tdata;

   tdata.a=10;
   tdata.b=32;

   pthread_create(&tid, NULL, myThread, (void *)&tdata);
   //pthread_join(tid, NULL);

   thread_data * arg_viene_de_exit;
   pthread_join(tid, (void**)&arg_viene_de_exit);

   thread_data de_exit = *(thread_data *)(arg_viene_de_exit); 

   printf("De Exit\n");
   printf("%d + %d = %d\n", arg_viene_de_exit->a, de_exit.b, de_exit.result); 
   printf("De Return\n");
   printf("%d + %d = %d\n", tdata.a, tdata.b, tdata.result);   
   free(arg_viene_de_exit);

   return 0;
}