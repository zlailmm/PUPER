/* Adapted from: https://svn.sosy-lab.org/software/sv-benchmarks/trunk/c/pthread/fib_bench_true-unreach-call.c */
extern void abort(void);
void reach_error(){}
void assume_abort_if_not(int cond) {
    if(!cond) {abort();}
}

#include <pthread.h>

int i=1; 
int j=1;

#define NUM 5

pthread_mutex_t l;

void *t1(void * arg)
{
  for (int k = 0; k < NUM; k++){
    // pthread_mutex_lock(l);
    i += j;
    // pthread_mutex_unlock(l);
  }
}

void *t2(void * arg)
{
  for (int k = 0; k < NUM; k++){
    // pthread_mutex_lock(l);
    j += i;
    // pthread_mutex_unlock(l);
  }
}

int
main()
{
  pthread_t id1;
  pthread_t id2;

  // pthread_mutex_init(l, NULL);

  pthread_create(&id1, NULL, t1, NULL);
  pthread_create(&id2, NULL, t2, NULL);

  pthread_join(id1,NULL);
  pthread_join(id2,NULL);

  int l=i;
  if (i > 144 || j > 144) {
      ERROR: {reach_error();abort();}
  } 
}
