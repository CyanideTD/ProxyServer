#ifndef _GLOBALSERV_H_
#define _GLOBALSERV_H_

#include "queue_t.h"
#include "Client.h"
#include "NetIO.h"
#include "DBth.h"

#define random(a, b) (((double)rand()/RAND_MAX)*(b-a)+a)
#define hash_map __gnu_cxx::hash_map

class NetIO;
class CWorkProcess;
class DBthread;

class GlobalServer
{
public:
    static GlobalServer*    g_GlobalServ;

public:

    static GlobalServer* Instance();
    static void Init(unsigned int threadsNum, int tasks);

    bool Start();
    void InitDatabase();

public:
    CWorkProcess*                   m_WorkProcessList;
    DBthread*                       m_DBthread;
    hash_map<int, int>*             m_LockFreqMap;
    CTaskQueue*                     m_WorkQue;
    CTaskQueue*                     m_RecvQue;
    int                             m_ProcessNum;
    NetIO*                          binary_receive;
    NetIO*                          http_receive;
    NetIO*                          net_send;                          
    CTaskQueue*                     m_cFreeSession;
    CTaskQueue*                     m_DBque;
};


#endif