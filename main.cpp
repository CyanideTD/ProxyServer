#include "GlobalServ.h"

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
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = RecvSignal;
    sigaction(SIGPIPE | SIGPROF, &sa, NULL);
}

int main()
{
    InitSignal();
    // signal(SIGPIPE, SIG_IGN);
    GlobalServer::g_GlobalServ->Init(500, 1000);
    GlobalServer::Instance()->Start();

    cout << tasks / 10;
}
