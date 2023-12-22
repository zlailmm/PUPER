/* Adapted from: https://svn.sosy-lab.org/software/sv-benchmarks/trunk/c/pthread-atomic/dekker_true-unreach-call.c */
extern void abort(void);
void reach_error(){}
void assume_abort_if_not(int cond) {
    if(!cond) {abort();}
}

#include <pthread.h>
#define assert(e) if (!(e)) ERROR: reach_error()

volatile int flag1 = 0; 
volatile int flag2 = 0; // boolean flags
volatile int turn = 0; // integer variable to hold the ID of the thread whose turn it is
volatile int x; // boolean variable to test mutual exclusion

void *thr1(void * arg) {
  flag1 = 1;
  while (flag2 >= 1) {
    if (turn != 0) {
      flag1 = 0;
      while (turn != 0) {};
      flag1 = 1;
    }
  }
  x = 0;
  if(x>0){
      reach_error();    //  assert(x<=0);
      abort();
  }
  turn = 1;
  flag1 = 0;
  return 0;//  return NULL;
}

void *thr2(void * arg) {
  flag2 = 1;
  while (flag1 >= 1) {
    if (turn != 1) {
      flag2 = 0;
      while (turn != 1) {}; //while (turn != 0) {};
      flag2 = 1;
    }
  }

  x = 1;
  if(x<1){
      reach_error();    //assert(x>=1);
      abort();
  }
  
  turn = 0;
  flag2 = 0;
  return 0;
}

int main() {

  pthread_t t1;
  pthread_t t2;
  assume_abort_if_not(0<=turn && turn<=1);
  pthread_create(&t1, NULL, thr1, NULL);
  pthread_create(&t2, NULL, thr2, NULL);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  return 0;
}
