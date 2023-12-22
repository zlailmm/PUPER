//
// Created by leo on 22-8-4.
//

extern void abort(void);
void reach_error(){}

#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void __VERIFIER_assert(int expression) { if (!expression) { ERROR: {reach_error();abort();}}; return; }

int a1,a2,a3,a4,a5,a6;

void *thread1(void * arg)
{
    a1=1;
    return 0;
}

void *thread2(void *arg)
{
    a2=1;
    return 0;
}

void *thread3(void *arg)
{
    a3=1;
    return 0;
}

void *thread4(void *arg)
{
    a4=1;
    return 0;
}

void *thread5(void *arg)
{
    a5=1;
    return 0;
}

void *thread6(void * arg)
{
    a6=1;
    return 0;
}

int main()
{
    pthread_t t1, t2 ,t3 ,t4 ,t5,t6;

    pthread_create(&t1, 0, thread1, 0);
    pthread_create(&t2, 0, thread2, 0);
    pthread_create(&t3, 0, thread3, 0);
    pthread_create(&t4, 0, thread4, 0);
    pthread_create(&t5, 0, thread5, 0);
    pthread_create(&t6, 0, thread6, 0);

    pthread_join(t1, 0);
    pthread_join(t2, 0);
    pthread_join(t3, 0);
    pthread_join(t4, 0);
    pthread_join(t5, 0);
    pthread_join(t6, 0);

    __VERIFIER_assert( a1 || a2 ||a3 ||a4||a5||a6);

    return 0;
}

