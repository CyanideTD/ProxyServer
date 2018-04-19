#ifndef _TASK_H
#define _TASK_H

#include <time.h>
#include <string.h>
#pragma pack(1)

struct LockNode
{
    int type;
    long  key;
};

#pragma pack()

typedef struct _Task
{
    LockNode* tasks;
    int length;
    struct timeval time;
    
    void Init(LockNode* a, int b)
    {
        tasks = a;
        length = b;
        memset(&time, 0, sizeof(time));
    }
    
    void Uninit()
    {
        memset(this, 0, sizeof(struct _Task));
    }

    ~_Task()
    {
        delete [] tasks;
    }

}Task;

bool operator<(const Task& t1, const Task& t2)
{
    if (t1.time.tv_sec == t2.time.tv_sec)
    {
        return t1.time.tv_usec < t2.time.tv_usec;
    }
    else
    {
        return t1.time.tv_sec < t2.time.tv_sec;
    }
}

bool operator>(const Task& t1, const Task& t2)
{
    if (t1.time.tv_sec == t2.time.tv_sec)
    {
        return t1.time.tv_usec > t2.time.tv_usec;
    }
    else
    {
        return t1.time.tv_sec > t2.time.tv_sec;
    }
}

#endif 