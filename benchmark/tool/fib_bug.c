/* Adapted from: https://svn.sosy-lab.org/software/sv-benchmarks/trunk/c/pthread/fib_bench_false-unreach-call.c */
extern void abort(void);
void reach_error(){}

#include <pthread.h>

int i=1, j=1;

#define NUM 5

void *t1(void * arg)
{
  for (int k = 0; k < NUM; k=k+1){
    i+=j;
  }
}

void *t2(void * arg)
{
  for (int k = 0; k < NUM; k=k+1){
    j += i;
  }


}

int
main(int argc, char **argv)
{

  pthread_t id1, id2;

  pthread_create(&id1, NULL, t1, NULL);
  pthread_create(&id2, NULL, t2, NULL);
  pthread_join(id1,NULL);
  pthread_join(id2,NULL);

  int l=i;
  if (l >= 144 || j >= 144) {
      ERROR: {reach_error();abort();}
  } 

}
