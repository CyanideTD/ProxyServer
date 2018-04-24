#include "Client.h"

using namespace std;

long glo_Seq = 0;
pthread_mutex_t seq_lock = PTHREAD_MUTEX_INITIALIZER;


CWorkProcess::CWorkProcess()
{
    pack = new CBaseProtocolPack;
    unPack = new CBaseProtocolUnpack;
    pack->Uninit();
    unPack->Uninit();
    pack->Init();
    unPack->Init();
}

CWorkProcess::~CWorkProcess()
{
    delete [] pack;
    delete [] unPack;
}

void CWorkProcess::init(CTaskQueue* work_queue, CTaskQueue* recvQue, ILongConn* send, LongConnHandle handle)
{
    m_dWorkQueue = work_queue;
    m_ReceQueue = recvQue;
    m_SendLongConn = send;
    LockServer = handle;
}



void* CWorkProcess::Start(TVOID* pParam)
{
    if (pParam != NULL)
    {
        CWorkProcess* pProcess = (CWorkProcess*) pParam;
        while (1)
        {
            pProcess->WorkRoutine();
        }
    }
    return NULL;
}

TVOID* CWorkProcess::WorkRoutine()
{
    SessionWrapper* session = 0;
    m_dWorkQueue->WaitTillPop(session);

    *pack = CBaseProtocolPack(session->m_szReqBuf, session->m_udwReqBufLen, session->m_udwReqBufLen);
    
    pthread_mutex_lock(&seq_lock);
    int seq = htonl(glo_Seq);
    glo_Seq++;
    pthread_mutex_unlock(&seq_lock);

    memcpy(&session->m_udwClientSeq,session->m_szReqBuf + 23, 4);
    memcpy(session->m_szReqBuf + 23, &seq, 4);
    session->m_GlobalSeq = ntohl(seq);

    LTasksGroup* stTasks = new LTasksGroup;
    stTasks->m_Tasks[0].SetConnSession(LockServer);
    stTasks->m_Tasks[0].SetSendData(session->m_szReqBuf, session->m_udwReqBufLen);
    stTasks->m_Tasks[0].SetNeedResponse(1);
    stTasks->SetValidTasks(1);
    m_SendLongConn->SendData(stTasks);

    m_ReceQueue->WaitTillPush(session);

}
