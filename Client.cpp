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
                        ILongConn* httpsend, ILongConn* lock, LongConnHandle lockserver)
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

TVOID* CWorkProcess::WorkRoutine()
{
    SessionWrapper* session = 0;
    m_dWorkQueue->WaitTillPop(session);



    if (session->m_bIsBinaryData)
    {
        
        LongConnHandle handle;
        bool needRes = false;
        ILongConn* conn = 0;
        if (session->m_sState == TO_LOCK)
        {
            handle = lock_server;
            needRes = true;
            conn = m_ILockConn;
        }
        else
        {
            handle = session->m_stHandle;
            conn = m_IBinaryRecvConn;
        }

        LTasksGroup stTasks;
        stTasks.m_UserData1.ptr = session;
        stTasks.m_Tasks[0].SetConnSession(handle);
        stTasks.m_Tasks[0].SetSendData(session->m_szData, session->m_udwBufLen);
        stTasks.m_Tasks[0].SetNeedResponse(needRes);
        stTasks.SetValidTasks(1);
        
        conn->SendData(&stTasks);
        
        if (session->m_sState == TO_LOCK)
        {
            // m_ReceQueue->WaitTillPush(session);
        }
        else
        {
            session->Reset();
            g_lNodeMgr.WaitTillPush(session);
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
                LockNode node;
                node.key = k;
                node.type = t;
                memcpy(buf + num * sizeof(node), &node, sizeof(node));
                num++;
            } while (keys && types);

            pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, (TUINT32)num);
            pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, buf, num * sizeof(LockNode));
            TUCHAR* pucPackage = NULL;
            TUINT32 udwPackageLen = 0;
            pack->GetPackage(&pucPackage, &udwPackageLen);

            LTasksGroup stTasks;
            stTasks.m_Tasks[0].SetConnSession(lock_server);
            stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
            stTasks.m_Tasks[0].SetNeedResponse(1);
            stTasks.SetValidTasks(1);
            stTasks.m_UserData1.ptr = session;
            m_ILockConn->SendData(&stTasks);

            // m_ReceQueue->WaitTillPush(session);


            delete [] buf;
        }
        else
        {
            unPack->UntachPackage();
            unPack->AttachPackage(session->m_szData, session->m_udwBufLen);
            unPack->Unpack();
            TINT32 ret;
            unPack->GetVal(EN_GLOBAL_KEY__RES_CODE, &ret);

            char* buf = new char[10240];
            TUINT32 length = 0;
            char* response = "HTTP/1.1 200 OK\r\n";
            char* content_type = "Content-Type:text/html\r\n\r\n";
            char* body;
            if (ret == 0)
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
    }

}
