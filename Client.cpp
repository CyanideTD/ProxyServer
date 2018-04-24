#include "Client.h"
#include <sstream>
#include <string>

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

void CWorkProcess::init(CTaskQueue* work_queue, CTaskQueue* recvQue, ILongConn* send, LongConnHandle handle, ILongConn* httpsend, LongConnHandle httphandle)
{
    m_dWorkQueue = work_queue;
    m_ReceQueue = recvQue;
    m_IBinaryLongConn = send;
    m_lBinaryLockServer = handle;
    m_IHttpLongConn = httpsend;
    m_LHttpLockServer = httphandle;
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
        *pack = CBaseProtocolPack(session->m_szReqBuf, session->m_udwReqBufLen, session->m_udwReqBufLen);
        
        pthread_mutex_lock(&seq_lock);
        int seq = htonl(glo_Seq);
        glo_Seq++;
        pthread_mutex_unlock(&seq_lock);

        memcpy(&session->m_udwClientSeq,session->m_szReqBuf + 23, 4);
        memcpy(session->m_szReqBuf + 23, &seq, 4);
        session->m_GlobalSeq = ntohl(seq);

        LTasksGroup* stTasks = new LTasksGroup;
        stTasks->m_Tasks[0].SetConnSession(m_lBinaryLockServer);
        stTasks->m_Tasks[0].SetSendData(session->m_szReqBuf, session->m_udwReqBufLen);
        stTasks->m_Tasks[0].SetNeedResponse(1);
        stTasks->SetValidTasks(1);
        m_IBinaryLongConn->SendData(stTasks);

        m_ReceQueue->WaitTillPush(session);
    } 
    else
    {
        std::string request((char*)session->m_szReqBuf, session->m_udwReqBufLen);
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
        stTasks.m_Tasks[0].SetConnSession(m_LHttpLockServer);
        stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
        stTasks.m_Tasks[0].SetNeedResponse(1);
        stTasks.SetValidTasks(1);
        m_IHttpLongConn->SendData(&stTasks);

        m_ReceQueue->WaitTillPush(session);


        delete [] buf;
    }

}
