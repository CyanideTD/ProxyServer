#include <iostream>
#include <pthread.h>
#include <sys/time.h>

using namespace std;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
volatile long long test = 0;

void* adder(void* pragm)
{
    int* count = new int[1];
    struct timeval start;
    gettimeofday(&start, NULL);
    struct timeval cur;
    while (1)
    {
        // pthread_mutex_lock(&mutex);
        test++;
        // pthread_mutex_unlock(&mutex);
        count[0] += 1;
        gettimeofday(&cur, NULL);
        if (cur.tv_sec - start.tv_sec > 1)
        {
            break;
        }
    }
    pthread_exit((void*) count);
}

int main()
{
    pthread_t* thId = new pthread_t[4];
    for (int i = 0; i < 4; i++)
    {
        pthread_create(&thId[i], NULL, adder, NULL);
    }

    void* nRes;
    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        pthread_join(thId[i], &nRes);
        count += ((int*)nRes)[0];
    }

    cout << count << endl << test;
    
}