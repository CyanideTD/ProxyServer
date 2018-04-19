#include "Client.h"

using namespace std;

int session = 0;
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

void CWorkProcess::init(LockNode* nodes, int nudeNum, RelationDataServiceType type)
{
    this->nodes = nodes;
    this->nodeNum = nudeNum;
    this->serviceType = type;
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
        // cout << "connect success" << endl;
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
        gettimeofday(&pProcess->start, NULL);
        // while (true)
        // {
        //     if (pProcess->serviceType == EN_SERVICE_TYPE_HU2LOCK__GET_REQ)
        //     {
        //         // pProcess->GetLock(&endSession);
        //     }
        //     else 
        //     {
        //         pProcess->ReleaseLock(&endSession);
        //     }
        //     if (errno == SIGPIPE || endSession)
        //     {
        //         pProcess->EndSession();
        //         return NULL;
        //     }
        //     // gettimeofday(&pProcess->end, NULL);
        //     // if (pProcess->end.tv_sec - pProcess->start.tv_sec > 10)
        //     // {
        //     //     break;
        //     // }
        //     pthread_testcancel();
        // }
        while (true)
        {
            pProcess->GetLock(&endSession);
            if (errno == SIGPIPE || endSession)
            {
                pProcess->EndSession();
                return NULL;
            }
            pProcess->ReleaseLock(&endSession);
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
        pack->ResetContent();
        pack->SetSeq(seq);
        TUINT32 timeout = 1000000;
        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, nodeNum);
        pack->SetKey(EN_KEY_HU2LOCK__REQ_TIMEOUT_US, timeout);

        memcpy(buf, nodes, sizeof(LockNode) * nodeNum);

        pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, buf, sizeof(LockNode) * nodeNum);
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
        }
        else
        {
            cout << "lock failed " << ret << endl;    
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
    pack->ResetContent();
    pack->SetSeq(seq);
    pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, nodeNum);
        
    TUCHAR* buf = new TUCHAR[1024];

    memcpy(buf, nodes, sizeof(LockNode) * nodeNum);

    pack->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, buf, sizeof(LockNode) * nodeNum);
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
    }
    else 
    {
        cout << "release failed " << ret << endl;    
    }

    // }
}

