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

    // _Task& operator=(const _Task& task)
    // {
    //     if (this != &task)
    //     {
    //         tasks = task.tasks;
    //         length = task.length;
    //         memcpy(&time, &task.time, sizeof(time));
    //     }
    // }

    friend bool operator>(_Task t1, _Task t2)
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

    ~_Task()
    {
    }

}Task;

struct Cmp
{
    bool operator() (const Task* t1, const Task* t2)
    {
        if (t1->time.tv_sec == t2->time.tv_sec)
        {
            return t1->time.tv_usec > t2->time.tv_usec;
        }
        else
        {
            return t1->time.tv_sec > t2->time.tv_sec;
        }
    }
};

static bool operator<(const struct timeval& t1, const struct timeval& t2)
{
    if (t1.tv_sec == t2.tv_sec)
    {
        return t1.tv_usec < t2.tv_usec;
    }
    return t1.tv_sec < t2.tv_sec;
}

// static bool operator>(const Task& t1, const Task& t2)
// {
//     if (t1.time.tv_sec == t2.time.tv_sec)
//     {
//         return t1.time.tv_usec > t2.time.tv_usec;
//     }
//     else
//     {
//         return t1.time.tv_sec > t2.time.tv_sec;
//     }
// }

#endif 