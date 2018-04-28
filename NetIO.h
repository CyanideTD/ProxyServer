#ifndef _NETIO_H_
#define _NETIO_H_

#include "GlobalServ.h"
#include "Session.h"

class NetIO : ITasksGroupCallBack
{
public:
    NetIO();
    ~NetIO();

    int Init(TCHAR* pszIp, TUINT16 uwPort, CTaskQueue* poWorkQueue, CTaskQueue* poRecvQue, bool bIsHttpListten, bool isConnToLock);

    int Uninit();

public:

    static TVOID*   RoutineNetIO(TVOID* pParam);

    TVOID StartNetServ();
    TVOID StopNetServ();

    virtual TVOID OnTasksFinishedCallBack(LTasksGroup* pstTasksGrp);

    virtual TVOID OnUserRequest(LongConnHandle stHandle, const TUCHAR* pszData, TUINT32 udwDataLen, TINT32 &dwWillResponse);

    TVOID StopLongConn();


public:
    
    SOCKET CreateListenSocket(TCHAR* pszListenHost, TUINT16 uwPort);

    SOCKET CloseListenSocket();

public:

    ILongConn*             m_poLongConn;
    CTaskQueue*            m_poWorkQueue;
    TINT32                 m_hListenSock;
    CBaseProtocolPack*     m_poPack;
    CBaseProtocolUnpack*   m_poUnpack;
    TCHAR                  m_szIp[4];
    TUINT16                m_uwPort;
    CTaskQueue*            m_ReceQue;
    LongConnHandle         m_uLockServer;
    bool                   m_bIsHttpListen;
    bool                   m_bIsConnToServ;

};

#endif