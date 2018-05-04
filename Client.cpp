#include "Client.h"
#include <sstream>
#include <string>

using namespace std;

long glo_Seq = 0;
pthread_mutex_t seq_lock = PTHREAD_MUTEX_INITIALIZER;
extern CTaskQueue    g_lNodeMgr;
extern CQueue<Resources*>    g_lRescNodeMgr;

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
    m_dbQue = dbque;
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

void CWorkProcess::ParseBinaryReqPackage(SessionWrapper* session, ProxyData* data, bool* bIsContinue)
{

    Resources* node = 0;
    g_lRescNodeMgr.WaitTillPop(node);
    unPack->UntachPackage();
    unPack->AttachPackage(session->m_szData, session->m_udwBufLen);
    unPack->Unpack();

    session->m_udwClientSeq = unPack->GetSeq();
    TUINT32 len = 0;

    ResourceNode test;

    unPack->GetVal(EN_KEY_RESOURCE_NUM, &node->num);

    TUCHAR* pszValBuf = 0;

    unPack->GetVal(EN_KEY_RESOURCE_LIST, &pszValBuf, &len);

    memcpy(&node->nodes, pszValBuf, sizeof(ResourceNode) * node->num);

    if ((len / sizeof(ResourceNode)) != node->num)
    {
        printf("Invalid node nums\n");
        session->m_retCode = 200;
        session->m_sState = SEND_BACK;
        return;
    }

    node->serviceType = unPack->GetServiceType();

    for (int i = 0; i < node->num; i++)
    {
        data->lock_list[i].type = 1;
        data->lock_list[i].key = node->nodes[i].UUID;
    }



    data->lock_num = node->num;
    data->serviceType = EN_SERVICE_TYPE_HU2LOCK__GET_REQ;
    session->m_sState = GET_LOCK;
    session->ptr = node;
}

void CWorkProcess::ParseResPackage(SessionWrapper* session, ProxyData* data, bool* bIsContinue)
{
    unPack->UntachPackage();
    unPack->AttachPackage(session->m_szData, session->m_udwBufLen);
    unPack->Unpack();
    data->_retCode = -1;
    unPack->GetVal(EN_GLOBAL_KEY__RES_CODE, &data->_retCode);

    if (data->_retCode == 0)
    {
        m_dbQue->WaitTillPush(session);
    }
    else
    {
        session->m_retCode = data->_retCode;
        session->m_sState = SEND_BACK;
        m_dWorkQueue->WaitTillPush(session);
    }
}

void CWorkProcess::ParseTextPackage(SessionWrapper* session, ProxyData* data, bool* bIsContinue)
{

    Resources* node = 0;
    g_lRescNodeMgr.WaitTillPop(node);
    std::string request((char*)session->m_szData, session->m_udwBufLen);
    std::istringstream iss(request);
    string method;
    string query;
    string protocol;

    if (!(iss >> method >> query >> protocol))
    {
        cout << "ERROR: parsing request\n";
        session->m_retCode = 300;
        session->m_sState = SEND_BACK;
        return;
    }

    iss.clear();
    iss.str(query);

    string url;

    if (!getline(iss, url, '?'))
    {
        cout << "ERROR: parsing request\n";
        cout << "ERROR: parsing request\n";
        session->m_retCode = 300;
        session->m_sState = SEND_BACK;
        return;
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

    if (params["type"] == "" || params["num"] == "" || params["UUID"]== "" || params["service"] == "")
    {
        cout << "Invalid Input\n";
        session->m_retCode = 300;
        session->m_sState = SEND_BACK;
        return;
    }

    // cout << "protocol: " << protocol << "\n";
    // cout << "method  : " << method << "\n";
    // cout << "url     :" << url << "\n";

    // cout << "type    : " << params["type"] << "\n";
    // cout << "num     : " << params["num"] << "\n";
    // cout << "UUID    : " << params["UUID"] << "\n";

    pack->ResetContent();
    pack->SetServiceType((TUINT32)atoi(params["service"].c_str()));

    data->lock_list[0].type = 1;
    data->lock_list[0].key = atoi(params["UUID"].c_str());
    
    data->lock_num = 1;
    data->serviceType = EN_SERVICE_TYPE_HU2LOCK__GET_REQ;

    node->num = 1;
    node->nodes[0].UUID = atoi(params["UUID"].c_str());
    node->nodes[0].type = atoi(params["type"].c_str());
    node->nodes[0].num = atoi(params["num"].c_str());
    node->serviceType = atoi(params["service"].c_str());
    session->m_sState = GET_LOCK;

    session->ptr = node;
}

void CWorkProcess::ParsePackage(SessionWrapper* session, ProxyData* data, bool* isContinue)
{
    if (session->m_sState == GET_REQ)
    {
        if (session->m_bIsBinaryData)
        {
            ParseBinaryReqPackage(session, data, isContinue);
        }
        else
        {
            ParseTextPackage(session, data, isContinue);
        }
    }
    else
    {

        ParseResPackage(session, data, isContinue);
    }
}

void CWorkProcess::SendToServer(SessionWrapper* session, ProxyData* data, bool* isContinue)
{

    if(data->serviceType == EN_SERVICE_TYPE_RESOURCE_COST || data->serviceType == EN_SERVICE_TYPE_RESOURCE_GET)
    {
        *isContinue = false;
    }

    pack->ResetContent();
    pthread_mutex_lock(&seq_lock);
    int seq = glo_Seq;
    glo_Seq++;
    pthread_mutex_unlock(&seq_lock);

    data->seq = seq;
    pack->SetSeq(seq);
    int needResponse = 0;

    if (session->m_sState == GET_LOCK)
    {
        pack->SetServiceType(EN_SERVICE_TYPE_HU2LOCK__GET_REQ);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, data->lock_num);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, (TUCHAR*)data->lock_list, sizeof(LockNode) * data->lock_num);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_TIMEOUT_US, data->timeout);
        session->m_sState = GET_RES;
        needResponse = 1;
    }
    else
    {
        pack->SetServiceType(EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, data->lock_num);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, (TUCHAR*)data->lock_list, sizeof(LockNode) * data->lock_num);
        session->m_sState = SEND_BACK;
    }

    TUCHAR* pucPackage = NULL;
    TUINT32 udwPackageLen = 0;

    pack->GetPackage(&pucPackage, &udwPackageLen);

    LTasksGroup stTasks;
    stTasks.m_Tasks[0].SetConnSession(lock_server);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(needResponse);
    stTasks.SetValidTasks(1);

    stTasks.m_UserData1.ptr = session;
    // session->m_bIsBinaryData = true;
    if (m_ILockConn->SendData(&stTasks) == FALSE)
    {
        session->m_sState = SEND_BACK;
        session->m_retCode = 200;
        *isContinue = true;
    }
}

void CWorkProcess::SendPackage(SessionWrapper* session, ProxyData* data)
{
    pack->ResetContent();

    pack->SetSeq(session->m_udwClientSeq);

    pack->SetKey(EN_GLOBAL_KEY__RES_CODE, session->m_retCode);

    TUCHAR *pucPackage = NULL;
	TUINT32 udwPackageLen = 0;

    pack->GetPackage(&pucPackage, &udwPackageLen);


    ILongConn* conn = m_IBinaryRecvConn;
    LongConnHandle handle = session->m_stHandle;

    LTasksGroup        stTasks;
	stTasks.m_Tasks[0].SetConnSession(handle);
	stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
	stTasks.m_Tasks[0].SetNeedResponse(0);
	stTasks.SetValidTasks(1);

    Resources* node = (Resources*) session->ptr;
    node->Reset();
    g_lRescNodeMgr.WaitTillPush(node);

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
    if (session->m_retCode == 0)
    {
        body = "<h1> succeed <h1>";
    }
    else
    {
        body = "<h1> failed <h1>";
    }
    char* encoding = "Content-Encoding:application\r\n";
    char* header = "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />";

    memcpy(buf + length, response, strlen(response));
    length += strlen(response);

    memcpy(buf + length, encoding, strlen(encoding));
    length += strlen(encoding);

    memcpy(buf + length, content_type, strlen(content_type));
    length += strlen(content_type);

    memcpy(buf + length, header, strlen(header));
    length += strlen(header);
    
    memcpy(buf + length, body, strlen(body));
    length += strlen(body);
 
    printf(buf, "%s");

    int sock = m_IHttpRecvConn->GetSockHandle(session->m_stHandle);
    
    send(sock, buf, length, 0);

    m_IHttpRecvConn->RemoveLongConnSession(session->m_stHandle);
    Resources* node = (Resources*) session->ptr;
    node->Reset();
    g_lRescNodeMgr.WaitTillPush(node);

    session->Reset();
    g_lNodeMgr.WaitTillPush(session);
    delete [] buf;
}

TVOID CWorkProcess::WorkRoutine()
{
    SessionWrapper* session = 0;
    m_dWorkQueue->WaitTillPop(session);

    ProxyData data;
    bool isContinue = true;

    if (session->m_sState == GET_REQ)
    {
        ParsePackage(session, &data, &isContinue);
        if (!isContinue)
        {
            return;
        }
    }

    if (session->m_sState == GET_LOCK)
    {
        SendToServer(session, &data, &isContinue);
        if (!isContinue)
        {
            return;
        }
    }

    if (session->m_sState == GET_RES)
    {
        ParsePackage(session, &data, &isContinue);
        if (!isContinue)
        {
            return;
        }
    }


    if (session->m_sState == SEND_UNLOCK)
    {
        SendToServer(session, &data, &isContinue);
        if (!isContinue)
        {
            return;
        }
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

void CWorkProcess::EncounterError(SessionWrapper* session)
{
    if (session->m_stHandle != lock_server)
    {
        if (session->m_bIsBinaryData)
        {
            m_IBinaryRecvConn->RemoveLongConnSession(session->m_stHandle);
        }
        else
        {
            m_IHttpRecvConn->RemoveLongConnSession(session->m_stHandle);
        }
    }

    Resources* node = (Resources*) session->ptr;
    node->Reset();
    g_lRescNodeMgr.WaitTillPush(node);

    session->Reset();
    g_lNodeMgr.WaitTillPush(session);
}
