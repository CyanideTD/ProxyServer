#include "Client.h"
#include <sstream>
#include <string>

using namespace std;

long glo_Seq = 0;
pthread_mutex_t seq_lock = PTHREAD_MUTEX_INITIALIZER;
extern CTaskQueue    g_lNodeMgr;

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

void CWorkProcess::init(CTaskQueue* taskQueue, CTaskQueue* recvQue, ILongConn* send, 
                        ILongConn* httpsend, ILongConn* lock, LongConnHandle lockserver, CTaskQueue* dbque)
{
    m_dWorkQueue = taskQueue;
    m_ReceQueue = recvQue;
    m_IBinaryRecvConn = send;
    m_IHttpRecvConn = httpsend;
    m_ILockConn = lock;
    lock_server = lockserver;
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

void CWorkProcess::ParseBinaryReqPackage(SessionWrapper* session, ResourceNode* node, ProxyData* data)
{
    unPack->UntachPackage();
    unPack->AttachPackage(session->m_szData, session->m_udwBufLen);
    unPack->Unpack();
    session->m_udwClientSeq = unPack->GetSeq();
    TUINT32 len = 0;
    unPack->GetVal(EN_KEY_RESOURCE, (TUCHAR**)&node, &len);
    node->serviceType = unPack->GetServiceType();

    data->lock_list[0].type = 1;
    data->lock_list[0].key = node->UUID;



    data->lock_num = 1;
    data->serviceType = EN_SERVICE_TYPE_HU2LOCK__GET_REQ;
    session->m_sState = GET_LOCK;
}

void CWorkProcess::ParseResPackage(SessionWrapper* session, ResourceNode* node, ProxyData* data)
{
    unPack->UntachPackage();
    unPack->AttachPackage(session->m_szData, session->m_udwBufLen);
    unPack->Unpack();
    unPack->GetVal(EN_GLOBAL_KEY__RES_CODE, &data->_retCode);

    if (data->_retCode == 0)
    {
        m_dbQue->WaitTillPush(session);
    }
    else
    {

    }
}

void CWorkProcess::ParseTextPackage(SessionWrapper* session, ResourceNode* node, ProxyData* data)
{
    std::string request((char*)session->m_szData, session->m_udwBufLen);
    std::istringstream iss(request);
    string method;
    string query;
    string protocol;

    if (!(iss >> method >> query >> protocol))
    {
        cout << "ERROR: parsing request\n";
    }

    iss.clear();
    iss.str(query);

    string url;

    if (!getline(iss, url, '?'))
    {
        cout << "ERROR: parsing request\n";
    }

    map<string, string> params;
    string keyval, key, val;
    while (getline(iss, keyval, '&'))
    {
        istringstream iss(keyval);
        if (getline(getline(iss, key, '='), val))
        {
            params[key] = val;
        }
    }

    cout << "protocol: " << protocol << "\n";
    cout << "method  : " << method << "\n";
    cout << "url     :" << url << "\n";

    cout << "params  : " << params["type"] << "\n";
    cout << "params  : " << params["num"] << "\n";
    cout << "params  : " << params["UUID"] << "\n";

    pack->ResetContent();
    pack->SetServiceType((TUINT32)atoi(params["service"].c_str()));

    data->lock_list[0].type = 1;
    data->lock_list[0].key = atoi(params["UUID"].c_str());
    
    data->lock_num = 1;
    data->serviceType = EN_SERVICE_TYPE_HU2LOCK__GET_REQ;

    node->UUID = atoi(params["UUID"].c_str());
    node->type = atoi(params["type"].c_str());
    node->num = atoi(params["num"].c_str());
    session->m_sState = GET_LOCK;
}

void CWorkProcess::ParsePackage(SessionWrapper* session, ResourceNode* node, ProxyData* data)
{
    if (session->m_bIsBinaryData)
    {
        if (session->m_sState == GET_REQ)
        {
            ParseBinaryReqPackage(session, node, data);
        }
        else
        {
            ParseResPackage(session, node, data);
        }
    }
    else
    {

        ParseTextPackage(session, node, data);
    }
}

void CWorkProcess::SendToServer(SessionWrapper* session, ResourceNode* node, ProxyData* data)
{
    pack->ResetContent();
    pthread_mutex_lock(&seq_lock);
    int seq = glo_Seq;
    glo_Seq++;
    pthread_mutex_unlock(&seq_lock);

    data->seq = seq;
    pack->SetSeq(seq);

    if (session->m_sState == GET_LOCK)
    {
        pack->SetServiceType(EN_SERVICE_TYPE_HU2LOCK__GET_REQ);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, data->lock_num);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, (TUCHAR*)data->lock_list, sizeof(LockNode) * data->lock_num);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_TIMEOUT_US, data->timeout);
    }
    else
    {
        pack->SetServiceType(EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, data->lock_num);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, (TUCHAR*)data->lock_list, sizeof(LockNode) * data->lock_num);
    }

    TUCHAR* pucPackage = NULL;
    TUINT32 udwPackageLen = 0;

    pack->GetPackage(&pucPackage, &udwPackageLen);

    LTasksGroup stTasks;
    stTasks.m_Tasks[0].SetConnSession(lock_server);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(1);
    stTasks.SetValidTasks(1);

    session->ptr = node;
    stTasks.m_UserData1.ptr = session;
    m_ILockConn->SendData(&stTasks);
}

void CWorkProcess::SendPackage(SessionWrapper* session, ProxyData* data)
{
    pack->ResetContent();

    pack->SetSeq(session->m_udwClientSeq);
    pack->SetKey(EN_GLOBAL_KEY__RES_CODE, data->_retCode);

    TUCHAR *pucPackage = NULL;
	TUINT32 udwPackageLen = 0;
    pack->SetServiceType(data->serviceType);
    pack->GetPackage(&pucPackage, &udwPackageLen);


    ILongConn* conn = m_IBinaryRecvConn;
    LongConnHandle handle = session->m_stHandle;
    int i = 0;

    conn = m_ILockConn;
    handle =lock_server;


    LTasksGroup        stTasks;
	stTasks.m_Tasks[0].SetConnSession(handle);
	stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
	stTasks.m_Tasks[0].SetNeedResponse(i);
	stTasks.SetValidTasks(1);
    stTasks.m_UserData1.ptr = session;


    session->Reset();
    g_lNodeMgr.WaitTillPush(session);

    conn->SendData(&stTasks);
}

void CWorkProcess::SendTextPackage(SessionWrapper* session, ProxyData* data)
{
    char* buf = new char[10240];
    TUINT32 length = 0;
    char* response = "HTTP/1.1 200 OK\r\n";
    char* content_type = "Content-Type:text/html\r\n\r\n";
    char* body;
    if (data->_retCode == 0)
    {
        body = "<h1> succeed <h1>";
    }
    else
    {
        body = "<h1> failed <h1>";
    }
    char* encoding = "Content-Encoding:application\r\n";
    char* header = "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />";

    strcat(buf, response);
    length += strlen(response);

    strcat(buf, encoding);
    length += strlen(encoding);

    strcat(buf, content_type);
    length += strlen(content_type);

    strcat(buf, header);
    length += strlen(header);
    
    strcat(buf, body);
    length += strlen(body);

    cout << buf;


    int sock = m_IHttpRecvConn->GetSockHandle(session->m_stHandle);
    
    send(sock, buf, length, 0);

    m_IHttpRecvConn->RemoveLongConnSession(session->m_stHandle);
    session->Reset();
    g_lNodeMgr.WaitTillPush(session);
    delete [] buf;
}

TVOID CWorkProcess::WorkRoutine()
{
    SessionWrapper* session = 0;
    m_dWorkQueue->WaitTillPop(session);
    ResourceNode* node = new ResourceNode;

    ProxyData data;
    bool isContinue = false;

    if (session->m_sState == GET_REQ)
    {
        ParsePackage(session, node, &data);
    }

    if (session->m_sState == GET_LOCK)
    {
        SendToServer(session, node, &data);
    }

    if (session->m_sState == GET_RES)
    {
        ParsePackage(session, node, &data);
    }


    if (session->m_sState == SEND_UNLOCK)
    {
        SendToServer(session, node, &data);
    }

    if (session->m_sState == SEND_BACK)
    {
        if (session->m_bIsBinaryData)
        {
            SendPackage(session, &data);
        }
        else
        {
            SendTextPackage(session, &data);
        }
    }
}
