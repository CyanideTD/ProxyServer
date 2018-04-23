#include <svrpublib/ServerPubLib.h>
#include <GlobalServ.h>
class NetIO : ITasksGroupCallBack
{
public:
    NetIO();
    ~NetIO();

    int Init(TCHAR* pszIp, TUINT16 uwPort, CQueue<Task*, Cmp>* poWorkQueue);

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

    ILongConn*           m_poLongConn;
    CQueue<Task*, Cmp>*  m_poWorkQueue;
    TINT32               m_hListenSock;
    CBaseProtocolPack*   m_poPack;
    CBaseProtocolUnpack*   m_poUnpack;
    TCHAR                m_szIp[4];
    TUINT16              m_uwPort;

};