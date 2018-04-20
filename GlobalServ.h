#ifndef _GLOBALSERV_H_
#define _GLOBALSERV_H_
#include "queue_t.h"
#include "Client.h"

#define random(a, b) (((double)rand()/RAND_MAX)*(b-a)+a)
#define hash_map __gnu_cxx::hash_map

class CWorkProcess;


class GlobalServer
{
public:
    static GlobalServer*    g_GlobalServ;

public:

    static GlobalServer* Instance();
    static bool Init(unsigned int threadsNum, int tasks);

    bool Start();

public:
    CWorkProcess*                   m_WorkProcessList;
    hash_map<int, int>*             m_LockFreqMap;
    CQueue<Task*, Cmp>*                  m_LockQue;
    CQueue<Task*, Cmp>*                  m_UnlockQue;
    int                             m_ProcessNum;
    struct Cmp                      cmp;
};


#endif