/* Testcase from Threader's distribution. For details see:
   http://www.model.in.tum.de/~popeea/research/threader

   This file is adapted from the example introduced in the paper:
   Thread-Modular Verification for Shared-Memory Programs
   by Cormac Flanagan, Stephen Freund, Shaz Qadeer.
*/
extern void abort(void);
void reach_error(){}

#include <pthread.h>
#include <assert.h>

int w=0, r=0, x, y;

void *writer1(void * arg) { //writer1
  int aux=w;
  while(aux != 0 || r != 0) { aux = w; } // __VERIFIER_assume(w==0 && r==0);
  w = 1;
  x = 3;
  w = 0;
}

void *writer2(void * arg) { //writer2
  int aux2=w;
  while(aux2 != 0 || r != 0) { aux2 = w; } // __VERIFIER_assume(w==0 && r==0);
  w = 1;
  x = 3;
  w = 0;
}

void *reader1(void * arg) { //reader1
  int l;

  int aux3=w;
  while(aux3 != 0){
    aux3 = w;
  }
  r = r+1;

  l = x;
  y = l;
  int aux4 = y;
  if(aux4 != x){
      reach_error();
      abort();
  }
  l = r-1;
  r = l;
}

void *reader2(void * arg) { //reader2
  int l2;

  int aux5=w;
  while(aux5 != 0){
    aux5 = w;
  }
  r = r+1;

  l2 = x;
  y = l2;
  int aux6 = y;
  if(aux6 != x){
      reach_error();
      abort();
  }
  l2 = r-1;
  r = l2;
}

int main() {
  pthread_t t1, t2, t3, t4;

  pthread_create(&t1, NULL, writer1, NULL);
  pthread_create(&t2, NULL, reader1, NULL);
  pthread_create(&t3, NULL, writer2, NULL);
  pthread_create(&t4, NULL, reader2, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  pthread_join(t3, NULL);
  pthread_join(t4, NULL);
}
