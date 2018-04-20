#include "GlobalServ.h"
#include <assert.h>
#include <time.h>
#include <semaphore.h>
#include <utility>
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
    
    g_GlobalServ->m_LockQue = new CQueue<Task*, Cmp>;
    g_GlobalServ->m_LockQue->Init(tasks);

    g_GlobalServ->m_UnlockQue = new CQueue<Task*, Cmp>;
    g_GlobalServ->m_UnlockQue->Init(tasks);

    g_GlobalServ->m_LockFreqMap = new hash_map<int, int>;
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
            while (g_GlobalServ->m_LockFreqMap->find(nodes[j].key) != g_GlobalServ->m_LockFreqMap->end())
            {
                nodes[j].key = random(1, 1000000);
            }
            g_GlobalServ->m_LockFreqMap->insert(std::make_pair(nodes[j].key, 1));
        }
        Task* task = new Task;
        task->Init(nodes, 2);
        g_GlobalServ->m_LockQue->WaitTillPush(task);
    }
    std::cout << "init finished" << std::endl;
}

bool GlobalServer::Start()
{
    pthread_t* thId = new pthread_t[g_GlobalServ->m_ProcessNum];
    for (int i = 0; i < g_GlobalServ->m_ProcessNum; i++)
    {
        pthread_create(&thId[i], NULL, CWorkProcess::Start, &g_GlobalServ->m_WorkProcessList[i]);
    }

    struct timeval start;
    gettimeofday(&start, NULL);

    while (1)
    {
        sleep(10);
        struct timeval cur;
        gettimeofday(&cur, NULL);
        std::cout << tasks/(cur.tv_sec - start.tv_sec) << std::endl;
    }

    for (int i = 0; i < g_GlobalServ->m_ProcessNum; i++)
    {
        pthread_join(thId[i], NULL);
    }

    delete [] thId;
}

