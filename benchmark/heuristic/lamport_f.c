/* Adapted from: https://svn.sosy-lab.org/software/sv-benchmarks/trunk/c/pthread-atomic/lamport_true-unreach-call.c */

/* Testcase from Threader's distribution. For details see:
   http://www.model.in.tum.de/~popeea/research/threader
*/
extern void abort(void);
void reach_error(){}

#include <pthread.h>


int x, y;
int b1, b2; // boolean flags
int X=1; // boolean variable to test mutual exclusion

void *thr1(void * arg) {
  while (1) {
    b1 = 1;
    x = 1;
    if (y != 0) {
      b1 = 0;
      while (y != 0) {}; // while(y!=0) {};
      continue; // continue;============
    }
    y = 1;
    if (x == 1) {
      b1 = 0;
      while (b2 >= 1) {}; // while (b2>=1) {};
      if (y != 1) {
          while (y != 0) {}; // while (y!=0) {};
          continue; //	continue;
      }
    }
      break;
  }
  // begin: critical section
  X = 0;
  if(X > 0){//  assert(X <= 0);
      reach_error();
      abort();
  }
  // end: critical section
  y = 0;
  b1 = 0;
}

void *thr2(void * arg) {
  while (1) {
    b2 = 1;
    x = 2;
    if (y != 0) {
      b2 = 0;
        while (y != 0) {};
        continue;
    }
    y = 2;
    if (x != 2) {
      b2 = 0;
        while (b1 >= 1) {}; // while (b1>=1) {};
      if (y != 2) {
          while (y != 0) {};
          continue; //	continue;
      }
    }
      break;
  }
  // begin: critical section
  X = 1;

  // end: critical section
  y = 0;
  b2 = 0;
}

int main() {
  pthread_t t1, t2;
  pthread_create(&t1, NULL, thr1, NULL);
  pthread_create(&t2, NULL, thr2, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
}
