#include "GlobalServ.h"
#include <assert.h>
#include <time.h>
#include <semaphore.h>
#include <utility>

GlobalServer* GlobalServer::g_GlobalServ = NULL;
CTaskQueue    g_lNodeMgr;
CQueue<ResourceNode*>    g_lRescNodeMgr;

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

void GlobalServer::Init(unsigned int threads, int fressNode)
{
    GlobalServer::Instance();

    g_lNodeMgr.Init(fressNode);
    for (int i = 0; i < fressNode; i++)
    {
        SessionWrapper* session = new SessionWrapper;
        session->Init();
        g_lNodeMgr.WaitTillPush(session);
    }

    g_lRescNodeMgr.Init(fressNode);
    for (int i = 0; i < fressNode; i++)
    {
        ResourceNode* node = new ResourceNode;
        node->Reset();
        g_lRescNodeMgr.WaitTillPush(node);
    }

    g_GlobalServ->m_ProcessNum = threads;
    g_GlobalServ->m_WorkProcessList = new CWorkProcess[threads];
    
    g_GlobalServ->m_RecvQue = new CTaskQueue;
    g_GlobalServ->m_RecvQue->Init(fressNode);

    g_GlobalServ->m_WorkQue = new CTaskQueue;
    g_GlobalServ->m_WorkQue->Init(fressNode);

    g_GlobalServ->m_cFreeSession = new CTaskQueue;
    g_GlobalServ->m_cFreeSession->Init(fressNode);

    g_GlobalServ->m_DBque = new CTaskQueue;
    g_GlobalServ->m_DBque->Init(fressNode);

    g_GlobalServ->m_DBthread = new DBthread;
    g_GlobalServ->m_DBthread->Init(g_GlobalServ->m_WorkQue, g_GlobalServ->m_DBque);

    g_GlobalServ->binary_receive = new NetIO;
    g_GlobalServ->binary_receive->Init("127.0.0.1", 16070, g_GlobalServ->m_WorkQue, g_GlobalServ->m_RecvQue, false, false);

    g_GlobalServ->http_receive = new NetIO;
    g_GlobalServ->http_receive->Init("127.0.0.1", 16080, g_GlobalServ->m_WorkQue, g_GlobalServ->m_RecvQue, true, false);

    g_GlobalServ->net_send = new NetIO;
    g_GlobalServ->net_send->Init("127.0.0.1", 16090, g_GlobalServ->m_WorkQue, g_GlobalServ->m_RecvQue, true, true);

    for (int i = 0; i < g_GlobalServ->m_ProcessNum; i++)
    {
        g_GlobalServ->m_WorkProcessList[i].init(g_GlobalServ->m_WorkQue, g_GlobalServ->m_RecvQue, 
                                                g_GlobalServ->binary_receive->m_poLongConn, g_GlobalServ->http_receive->m_poLongConn,
                                                g_GlobalServ->net_send->m_poLongConn, g_GlobalServ->net_send->m_uLockServer, g_GlobalServ->m_DBque);
    }

}

bool GlobalServer::Start()
{
    pthread_t* thId = new pthread_t[g_GlobalServ->m_ProcessNum];
    pthread_t  thIOId;
    
    pthread_create(&thIOId, NULL, NetIO::RoutineNetIO, g_GlobalServ->binary_receive);
    pthread_create(&thIOId, NULL, NetIO::RoutineNetIO, g_GlobalServ->http_receive);
    pthread_create(&thIOId, NULL, NetIO::RoutineNetIO, g_GlobalServ->net_send);

    pthread_create(&thIOId, NULL, DBthread::Start, g_GlobalServ->m_DBthread);

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
