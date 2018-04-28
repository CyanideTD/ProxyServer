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
                        ILongConn* httpsend, ILongConn* lock, LongConnHandle lockserver, sql::Connection* con)
{
    m_dWorkQueue = taskQueue;
    m_ReceQueue = recvQue;
    m_IBinaryRecvConn = send;
    m_IHttpRecvConn = httpsend;
    m_ILockConn = lock;
    lock_server = lockserver;
    m_dbConn = con;
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

void CWorkProcess::ParsePackage(SessionWrapper* session, ProxyData* data)
{
    if (session->m_bIsBinaryData || session->m_sState == SEND_BACK)
    {
        unPack->UntachPackage();
        unPack->AttachPackage(session->m_szData, session->m_udwBufLen);
        unPack->Unpack();
        data->seq = unPack->GetSeq();
        data->type = unPack->GetServiceType();
        if (session->m_sState == TO_LOCK)
        {
            unPack->GetVal(EN_KEY_HU2LOCK__REQ_KEY_NUM, &data->lock_num);
            TUINT32 bufLen = 0;
            TUCHAR* pszValBuf = 0;
            unPack->GetVal(EN_KEY_HU2LOCK__REQ_KEY_LIST, &pszValBuf, &bufLen);
            unPack->GetVal(EN_KEY_HU2LOCK__REQ_TIMEOUT_US, &data->timeout);
            session->m_udwClientSeq = unPack->GetSeq();
        }
        else
        {
            unPack->GetVal(EN_GLOBAL_KEY__RES_CODE, &data->_retCode);

        }
    }
    else
    {
        if (session->m_sState == TO_LOCK)
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

            cout << "params  : " << params["key"] << "\n";
            cout << "params  : " << params["type"] << "\n";

            istringstream keys(params["key"]);
            istringstream types(params["type"]);

            pack->ResetContent();
            pack->SetServiceType((TUINT32)atoi(params["service"].c_str()));
            
            pthread_mutex_lock(&seq_lock);
            int seq = glo_Seq;
            glo_Seq++;
            pthread_mutex_unlock(&seq_lock);

            pack->SetSeq(seq);
            TUINT32 num = 0;
            TUCHAR* buf = new TUCHAR[10240];

            do 
            {
                string _key;
                string _type;
                getline(keys, _key, '+');
                getline(types, _type, '+');
                if (_key == "" || _type == "")
                {
                    break;
                }
                int k = atoi(_key.c_str());
                int t = atoi(_type.c_str());
                data->lock_list[num].key = k;;
                data->lock_list[num].type = t;
                num++;
            } while (keys && types);

            data->lock_num = num;
            data->type = EN_SERVICE_TYPE_HU2LOCK__GET_REQ;
            data->timeout = atoi(params["timeout"].c_str());
        }
    }
}

void CWorkProcess::DoSomeThing(SessionWrapper* session, ProxyData* data)
{
    std::auto_ptr<sql::PreparedStatement> prep_stmt(m_dbConn->prepareStatement("SELECT * FROM test where Handle=(?)"));
    prep_stmt->setInt(1, session->m_stHandle.SerialNum);
    std::auto_ptr<sql::ResultSet> res(prep_stmt->executeQuery());
    if (res->next())
    {
        int num = res->getInt("Count");
        num++;
        std::auto_ptr<sql::PreparedStatement> prep_stmt(m_dbConn->prepareStatement("UPDATE test SET Count=(?) WHERE Handle=(?)"));
        prep_stmt->setInt(1, num);
        prep_stmt->setInt(2, session->m_stHandle.SerialNum);
        prep_stmt->execute();
    }
    else
    {
        std::auto_ptr<sql::PreparedStatement> prep_stmt(m_dbConn->prepareStatement("INSERT INTO test(Handle, Count) VALUE(?,1)"));
        prep_stmt->setInt(1, session->m_stHandle.SerialNum);
        prep_stmt->execute();
    }
}

void CWorkProcess::ProcessPackage(SessionWrapper* session, ProxyData* data)
{
    if (session->m_sState == TO_LOCK)
    {
        DoSomeThing(session, data);
    }
}

void CWorkProcess::SendPackage(SessionWrapper* session, ProxyData* data, bool* isContinue)
{
    pack->ResetContent();

    if (session->m_sState == TO_LOCK)
    {
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, data->lock_num);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, (TUCHAR*)data->lock_list, sizeof(LockNode) * data->lock_num);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_TIMEOUT_US, data->timeout);
        pthread_mutex_lock(&seq_lock);
        pack->SetSeq(glo_Seq++);
        pthread_mutex_unlock(&seq_lock);
    }
    else 
    {
        pack->SetSeq(session->m_udwClientSeq);
        pack->SetKey(EN_GLOBAL_KEY__RES_CODE, data->_retCode);
    }

    TUCHAR *pucPackage = NULL;
	TUINT32 udwPackageLen = 0;
    pack->SetServiceType(data->type);
    pack->GetPackage(&pucPackage, &udwPackageLen);


    ILongConn* conn = m_IBinaryRecvConn;
    LongConnHandle handle = session->m_stHandle;
    int i = 0;

    if (session->m_sState == TO_LOCK)
    {
        conn = m_ILockConn;
        handle =lock_server;
        i = 1;
    }

    LTasksGroup        stTasks;
	stTasks.m_Tasks[0].SetConnSession(handle);
	stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
	stTasks.m_Tasks[0].SetNeedResponse(i);
	stTasks.SetValidTasks(1);
    stTasks.m_UserData1.ptr = session;

    if (session->m_sState==TO_LOCK)
    {
    }
    else
    {
        session->Reset();
        g_lNodeMgr.WaitTillPush(session);
    }

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

TVOID* CWorkProcess::WorkRoutine()
{
    SessionWrapper* session = 0;
    m_dWorkQueue->WaitTillPop(session);

    ProxyData data;
    bool isContinue = false;

    ParsePackage(session, &data);

    ProcessPackage(session, &data);

    if (session->m_sState == SEND_BACK && !session->m_bIsBinaryData)
    {
        SendTextPackage(session, &data);
    }
    else
    {
        SendPackage(session, &data, &isContinue);
    }
}
