#include "Client.h"

using namespace std;

long long tasks = 0;

void RecvSignal(int signal)
{
    pthread_t tid = pthread_self();
    switch(signal)
    {
        case SIGPIPE:
            cout << tid <<" receive SIGPIPE" << endl;
            break;
    }
}

TVOID   InitSignal()
{
    struct sigaction sa;
    sigset_t sset;
    sa.sa_handler = RecvSignal;
    sigaction(SIGPIPE | SIGPROF, &sa, NULL);
}

int main()
{
    InitSignal();
    // signal(SIGPIPE, SIG_IGN);
    LockNode* nodes = new LockNode[10];
    
    for (int i = 0; i < 10; i++)
    {
        nodes[i].key = i;
        nodes[i].type = 1;
    }

    int num = 100;
    pthread_t thId[num];
    CWorkProcess* process = new CWorkProcess[num];

    for (int i = 0; i < num; i++){
        if (i % 5 == 0)
        {
            process[i].init(nodes, 10, EN_SERVICE_TYPE_HU2LOCK__GET_REQ);
        }
        // process[1].init(nodes, 10, EN_SERVICE_TYPE_HU2LOCK__GET_REQ);
        if (i % 5 != 0)
        {
            process[i].init(nodes, 10, EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ);
        }
        // process[3].init(nodes, 10, EN_SERVICE_TYPE_HU2LOCK__GET_REQ);
    }

    for (int i = 0; i < num; i++)
    {
        int ret = pthread_create(&thId[i], NULL, CWorkProcess::Start, &process[i]);
        if (ret != 0)
        {
            cout << "failed to create thread " << i << endl;
        }
    }
    
    // for (int i = 0; i < num; i++)
    // {
    //     pthread_cancel(thId[i]);
    // }

    for (int i = 0; i < num; i++)
    {
        pthread_join(thId[i], NULL);
    }

    cout << tasks / 10;
}
