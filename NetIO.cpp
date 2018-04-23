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

int NetIO::Init(TCHAR* pszIp, TUINT16 uwPort, CQueue<Task*, Cmp>* poWorkQueue)
{
    m_poWorkQueue = poWorkQueue;
    m_uwPort = uwPort;
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

    if (m_poLongConn->InitLongConn(this, 1024, m_hListenSock, 100U) == FALSE)
    {
        return -1;
    }

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
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return -1;
    }

    struct sockaddr_in host;
    host.sin_port = udwPort;
    inet_aton(pszListenHost, &host.sin_addr);
    host.sin_family = AF_INET;
    int flag = bind(sock, (struct sockaddr*)&host, sizeof(host));
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
    
}


