#include "Client.h"

using namespace std;

int session = 0;
pthread_mutex_t map_lock = PTHREAD_MUTEX_INITIALIZER;

int IsLockMap(bool isLock)
{
    int flag = 0;
    if (isLock)
    {
        flag = pthread_mutex_lock(&map_lock);
    }
    else
    {
        flag = pthread_mutex_unlock(&map_lock);
    }

    if (flag == -1)
    {
        pthread_mutex_unlock(&map_lock);
    }
    return flag;
}

CWorkProcess::CWorkProcess()
{
    pack = new CBaseProtocolPack;
    unPack = new CBaseProtocolUnpack;
    pack->Uninit();
    unPack->Uninit();
    pack->Init();
    unPack->Init();
    seq = 0;
    buf = new TUCHAR[1024];
}

CWorkProcess::~CWorkProcess()
{
    delete [] pack;
    delete [] unPack;
    delete [] buf;
}

void CWorkProcess::init(hash_map<int, int>* map, CQueue<Task*, Cmp>* lock, CQueue<Task*, Cmp>* unlock)
{
    this->serviceType = EN_SERVICE_TYPE_HU2LOCK__GET_REQ;
    m_LockFreqMap = map;
    m_LockQue = lock;
    m_UnlockQue = unlock;
}

void CWorkProcess::EndSession()
{
    cout << "End Session" << endl;
    close(sockfd);
}

bool CWorkProcess::connectTo()
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    inet_aton("127.0.0.1", &server.sin_addr);
    server.sin_port = htons(16060);
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        cout << "invalid socket" << endl;
    }

    int i = connect(sock, (struct sockaddr*)&server, sizeof(server));
    if (i != -1)
    {
        cout << "connect success" << endl;
        session++;
        sockfd = sock;
        return true;
    }
    else
    {
        cout << "connect fail" << endl;
        return false;
    }
}

void* CWorkProcess::Start(TVOID* pParam)
{
    if (pParam != NULL)
    {
        CWorkProcess* pProcess = (CWorkProcess*) pParam;
        if (!pProcess->connectTo())
        {
            return NULL;
        }
        bool endSession = false;
        while (true)
        {
            if (pProcess->serviceType == EN_SERVICE_TYPE_HU2LOCK__GET_REQ)
            {
                pProcess->GetLock(&endSession);
            }
            else 
            {
                pProcess->ReleaseLock(&endSession);
            }
            if (errno == SIGPIPE || endSession)
            {
                pProcess->EndSession();
                return NULL;
            }
        }
    }
    return NULL;
}

TINT32 CWorkProcess::GetLock(bool* endSession)
{

    // while (true)
    // {
    int flag = 0;
    Task* task = 0;
    m_LockQue->WaitTillPop(task);

    // flag = IsLockMap(true);
    // if (flag == -1)
    // {
    //     m_LockQue->WaitTillPush(task);
    //     return -1;
    // }

    // for (int i = 0; i < task->length; i++)
    // {
    //     int key = task->tasks[i].key;
    //     while (m_LockFreqMap->find(key) == m_LockFreqMap->end() && (*m_LockFreqMap)[key] >= 2)
    //     {
    //         task->tasks[i].key = random(1, 1000000);
    //     }
    // }

    // IsLockMap(false);

    pack->ResetContent();
    pack->SetSeq(seq);
    TUINT32 timeout = 1000000;
    pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, task->length);
    pack->SetKey(EN_KEY_HU2LOCK__REQ_TIMEOUT_US, timeout);

    memcpy(buf, (char*) &task->tasks[0], sizeof(LockNode) * task->length);

    pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, buf, sizeof(LockNode) * task->length);
    pack->SetServiceType(EN_SERVICE_TYPE_HU2LOCK__GET_REQ);
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    pack->GetPackage(&pucPackage, &udwPackageLen);

    // while (true)
    // {
    int nbytes = -1;
    while (udwPackageLen > 0)
    {
        nbytes = send(sockfd, pucPackage, udwPackageLen, 0);
        udwPackageLen-= nbytes;
        pucPackage += nbytes;
    }
    if (nbytes <= 0) 
    {
        cout << "connection break";
        *endSession = true;
        return 0;
    }

    // flag = IsLockMap(true);
    // if (flag == -1)
    // {
    //     cout << "mutex failed" << endl;
    // }

    // for (int i = 0; i < task->length; i++)
    // {
    //     int key = task->tasks[i].key;
    //     if (m_LockFreqMap->find(key) == m_LockFreqMap->end())
    //     {
    //         m_LockFreqMap->insert(make_pair(key, 1));
    //     }
    //     else
    //     {
    //         (*m_LockFreqMap)[key] += 1;
    //     }
    // }

    // flag = IsLockMap(false);
    

    int rec = recv(sockfd, buf, 1024, 0);
    if (rec <= 0) 
    {
        cout << "connection break";
        *endSession = true;
        return 0;
    }
    else 
    {
        tasks++;
    }
    unPack->UntachPackage();
    unPack->AttachPackage(buf, rec);
    unPack->Unpack();

    TINT32 ret = -1;

    unPack->GetVal(EN_GLOBAL_KEY__RES_CODE, &ret);
    // delete [] buf;
    if (ret == 0)
    {
        // cout << "lock succeed" << endl;
        serviceType = EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ;
        gettimeofday(&(task->time), NULL);
        task->time.tv_sec += 0;
        m_UnlockQue->WaitTillPush(task);
    }
    else
    {
        cout << "lock failed " << ret << endl;
        m_LockQue->WaitTillPush(task);   
    }
    // }
    
    //cout << "recv: " << rec << endl;
    // seq++;
    // key++;
    // if (seq == INT32_MAX || key == INT32_MAX) {
    //     break;
    // }
    
// }
    
    return 0;
}

TINT32 CWorkProcess::ReleaseLock(bool* endSession)
{
    int flag = 0;

    Task* task = 0;
    m_UnlockQue->WaitTillPop(task);
    struct timeval cur;
    gettimeofday(&cur, NULL);
    if (cur < task->time)
    {
        m_UnlockQue->WaitTillPush(task);
        return 0;
    }
    pack->ResetContent();
    pack->SetSeq(seq);
    pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, task->length);
        
    TUCHAR* buf = new TUCHAR[1024];

    memcpy(buf, (char*)&task->tasks[0], sizeof(LockNode) * task->length);

    pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, buf, sizeof(LockNode) * task->length);
    pack->SetServiceType(EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ);
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    pack->GetPackage(&pucPackage, &udwPackageLen);

    // while (true)
    // {
    int nbytes = -1;
    while (udwPackageLen > 0)
    {
        nbytes = send(sockfd, pucPackage, udwPackageLen, 0);
        udwPackageLen-= nbytes;
        pucPackage += nbytes;
    }
    if (nbytes <= 0) 
    {
        cout << "connection break";
        *endSession = true;
        return 0;
    }
    

    int rec = recv(sockfd, buf, 1024, 0);
    if (rec <= 0) 
    {
        cout << "connection break";
        *endSession = true;;
        return 0;
    }
    else 
    {
        tasks++;
    }
    unPack->UntachPackage();
    unPack->AttachPackage(buf, rec);
    unPack->Unpack();

    TINT32 ret = -1;

    unPack->GetVal(EN_GLOBAL_KEY__RES_CODE, &ret);
    // delete [] buf;
    if (ret == 0)
    {
        // cout << "release succeed" << endl;
        serviceType = EN_SERVICE_TYPE_HU2LOCK__GET_REQ;
        m_LockQue->WaitTillPush(task);
    }
    else 
    {
        cout << "release failed " << ret << endl;    
    }

    // }
}

