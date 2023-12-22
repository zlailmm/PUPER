/* Adapted from PGSQL benchmark from http://link.springer.com/chapter/10.1007%2F978-3-642-37036-6_28 */

/* BOUND 8 */

//#include <stdbool.h>
//#include <assert.h>
extern void abort(void);
void reach_error(){}

#include <pthread.h>
#include <assert.h>

//void __VERIFIER_assume(int);

int latch1 = 1;
int flag1  = 1;
int latch2 = 0;
int flag2  = 0;

int __unbuffered_tmp2 = 0;

void* worker_1(void * arg)
{
  while(1) {
    // __VERIFIER_assume(latch1);
  while(latch1 != 1) {};
    //assert(!latch1 || flag1);
    if(latch1){
      if(!flag1){
          reach_error();
          abort();
      }
    }

    latch1 = 0;
    if(flag1) {
      flag1 = 0;
      flag2 = 1;
      latch2 = 1;
    }
  }
}

void* worker_2(void * arg)
{
  while(1) {
    //    __VERIFIER_assume(latch2);
  while(latch2 != 1) {};
    
    //    assert(!latch2 || flag2);
    if(latch2){
      if(!flag2){
          reach_error();
          abort();
      }
    }
    latch2 = 0;
    if(flag2) {
      flag2 = 0;
      flag1 = 1;
      latch1 = 1;
    }
  }
}

int main() {
  pthread_t t1;
  pthread_t t2;
  pthread_create(&t1, NULL, worker_1, NULL);
  pthread_create(&t2, NULL, worker_2, NULL);
}
