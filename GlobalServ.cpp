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

void GlobalServer::Init(unsigned int threads, int tasks)
{
    GlobalServer::Instance();

    g_GlobalServ->m_ProcessNum = threads;
    g_GlobalServ->m_WorkProcessList = new CWorkProcess[threads];
    
    g_GlobalServ->m_RecvQue = new CTaskQueue;
    g_GlobalServ->m_RecvQue->Init(tasks);

    g_GlobalServ->m_WorkQue = new CTaskQueue;
    g_GlobalServ->m_WorkQue->Init(tasks);


    g_GlobalServ->binary_net_io = new NetIO;
    g_GlobalServ->binary_net_io->Init("127.0.0.1", 16070, g_GlobalServ->m_WorkQue, g_GlobalServ->m_RecvQue, false);

    g_GlobalServ->http_net_io = new NetIO;
    g_GlobalServ->http_net_io->Init("127.0.0.1", 16080, g_GlobalServ->m_WorkQue, g_GlobalServ->m_RecvQue, true);

    for (int i = 0; i < g_GlobalServ->m_ProcessNum; i++)
    {
        g_GlobalServ->m_WorkProcessList[i].init(g_GlobalServ->m_WorkQue, g_GlobalServ->m_RecvQue, 
                                                g_GlobalServ->binary_net_io->m_poLongConn, g_GlobalServ->binary_net_io->m_uLockServer,
                                                g_GlobalServ->http_net_io->m_poLongConn, g_GlobalServ->http_net_io->m_uLockServer);
    }

}

bool GlobalServer::Start()
{
    pthread_t* thId = new pthread_t[g_GlobalServ->m_ProcessNum];
    pthread_t  thIOId;
    
    pthread_create(&thIOId, NULL, NetIO::RoutineNetIO, g_GlobalServ->binary_net_io);
    pthread_create(&thIOId, NULL, NetIO::RoutineNetIO, g_GlobalServ->http_net_io);

    for (int i = 0; i < g_GlobalServ->m_ProcessNum; i++)
    {
        pthread_create(&thId[i], NULL, CWorkProcess::Start, &g_GlobalServ->m_WorkProcessList[i]);
    }

    struct timeval start;
    gettimeofday(&start, NULL);

    for (int i = 0; i < g_GlobalServ->m_ProcessNum; i++)
    {
        pthread_join(thId[i], NULL);
    }

    delete [] thId;
}

