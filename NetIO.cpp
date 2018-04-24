#include "NetIO.h"

NetIO::NetIO()
{

}

NetIO::~NetIO()
{
    if (m_poLongConn != NULL)
    {
        m_poLongConn->UninitLongConn();
        m_poLongConn->Release();
        m_poLongConn = NULL;
    }

    m_poPack->Uninit();
    m_poUnpack->Uninit();
}

int NetIO::Init(TCHAR* pszIp, TUINT16 uwPort, CTaskQueue* poWorkQueue, CTaskQueue* poRecvQue, bool bIsHttpListen)
{
    m_bIsHttpListen = bIsHttpListen;
    m_poWorkQueue = poWorkQueue;
    m_ReceQue = poRecvQue;
    m_uwPort = htons(uwPort);
    TUINT32 ip = inet_addr(pszIp);
    memcpy(m_szIp, &ip, sizeof(TUINT32));

    m_poLongConn = CreateLongConnObj();
    if (m_poLongConn == NULL)
    {
        return -1;
    }

    m_hListenSock = CreateListenSocket(pszIp, uwPort);

    if (m_hListenSock < 0)
    {
        return -1;
    }

    if (m_poLongConn->InitLongConn(this, 1024, m_hListenSock, 100U, 0, 0, bIsHttpListen) == FALSE)
    {
        return -1;
    }

    m_uLockServer = m_poLongConn->CreateLongConnSession("127.0.0.1", 16060);

    m_poPack = new CBaseProtocolPack;
    m_poPack->Init();
    m_poUnpack = new CBaseProtocolUnpack;
    m_poUnpack->Init();

    return 1;
}

TVOID* NetIO::RoutineNetIO(TVOID* pParam)
{
    NetIO*  netProcess = 0;
    netProcess = (NetIO*) pParam;
    while (1)
    {
        netProcess->m_poLongConn->RoutineLongConn(1000);
    }
    return 0;
}

SOCKET NetIO::CreateListenSocket(TCHAR* pszListenHost, TUINT16 udwPort)
{
    int flag = 1;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
    if (sock < 0)
    {
        return -1;
    }

    struct sockaddr_in host;
    host.sin_port = htons(udwPort);
    inet_aton(pszListenHost, &host.sin_addr);
    host.sin_family = AF_INET;
    flag = bind(sock, (struct sockaddr*)&host, sizeof(host));
    if (flag < 0)
    {
        return -1;
    }

    flag = listen(sock, 1024);
    if (flag < 0)
    {
        return -1;
    }

    return sock;
}

int NetIO::CloseListenSocket()
{
    if (m_hListenSock >= 0)
    {
        close(m_hListenSock);
    }
    return 1;
}

TVOID NetIO::OnUserRequest(LongConnHandle stHandle, const TUCHAR* pszData, TUINT32 udwDataLen, TINT32 &dwWillResponse)
{
    dwWillResponse = true;
    SessionWrapper* session = new SessionWrapper;
    memcpy(session->m_szReqBuf, pszData, udwDataLen);
    session->m_stHandle = stHandle;
    session->m_udwReqBufLen = udwDataLen;
    session->m_bIsBinaryData = !m_bIsHttpListen;
    m_poWorkQueue->WaitTillPush(session);
}

TVOID NetIO::OnTasksFinishedCallBack(LTasksGroup* pstTasksGrp)
{
    SessionWrapper* session = 0;

    m_poUnpack->UntachPackage();
    m_poUnpack->AttachPackage(pstTasksGrp->m_Tasks[0]._pReceivedData, pstTasksGrp->m_Tasks[0]._uReceivedDataLen);
    m_poUnpack->Unpack();
    int seq = m_poUnpack->GetSeq();
    while (1)
    {
        m_ReceQue->WaitTillPop(session);
        if (session->m_GlobalSeq == seq)
        {
            break;
        }
        else {
            m_ReceQue->WaitTillPush(session);
        }
        
    }

    if (!m_bIsHttpListen)
    {
        int client_seq = session->m_udwClientSeq;
        memcpy(pstTasksGrp->m_Tasks[0]._pReceivedData + 23, &client_seq, 4);
    }
    
    LTasksGroup stTasks;
    stTasks.m_Tasks[0].SetConnSession(session->m_stHandle);
    stTasks.m_Tasks[0].SetSendData(pstTasksGrp->m_Tasks[0]._pReceivedData, pstTasksGrp->m_Tasks[0]._uReceivedDataLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);
    
    m_poLongConn->SendData(&stTasks);

    delete session;

}


