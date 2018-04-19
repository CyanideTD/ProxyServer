#include "GlobalServ.h"
#include <assert.h>
#include <time.h>
#include <semaphore.h>

GlobalServer* GlobalServer::g_GlobalServ = NULL;

GlobalServer* GlobalServer::Instance()
{
    if (g_GlobalServ == NULL)
    {
        g_GlobalServ = new GlobalServer;
        if (g_GlobalServ == NULL)
        {
            assert(0);
        }
    }
    return g_GlobalServ;
}

bool GlobalServer::Init(unsigned int threads, int tasks)
{
    GlobalServer::Instance();

    g_GlobalServ->m_ProcessNum = threads;
    g_GlobalServ->m_WorkProcessList = new CWorkProcess[threads];
    g_GlobalServ->m_LockQue = new CQueue<Task>;
    g_GlobalServ->m_LockQue->Init(tasks);

    g_GlobalServ->m_UnlockQue = new CQueue<Task>;
    g_GlobalServ->m_UnlockQue->Init(tasks);
    for (int i = 0; i < threads; i++)
    {
        g_GlobalServ->m_WorkProcessList[i].init(g_GlobalServ->m_LockFreqMap, g_GlobalServ->m_LockQue, g_GlobalServ->m_UnlockQue);
    }

    srand((int) time(0));
    for (int i = 0; i < tasks; i++)
    {
        LockNode* nodes = new LockNode[2];
        for (int j = 0; j < 2; j++)
        {
            nodes[j].type = 1;
            nodes[j].key = random(1, 1000000);
        }
        Task task;
        task.Init(nodes, 2);
        g_GlobalServ->m_LockQue->WaitTillPush(task);

    }

    g_GlobalServ->m_LockFreqMap = new hash_map<int, int>;
}

bool GlobalServer::Start()
{
    pthread_t* thId = new pthread_t[g_GlobalServ->m_ProcessNum];
    for (int i = 0; i < g_GlobalServ->m_ProcessNum; i++)
    {
        pthread_create(&thId[i], NULL, CWorkProcess::Start, &g_GlobalServ->m_WorkProcessList[i]);
    }

    for (int i = 0; i < g_GlobalServ->m_ProcessNum; i++)
    {
        pthread_join(thId[i], NULL);
    }

    delete [] thId;
    delete [] g_GlobalServ;
}

