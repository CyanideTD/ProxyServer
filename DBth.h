#include "std_header.h"
#include "queue_t.h"
#include "Task.h"
#include "Session.h"

class DBthread
{
private:
    sql::Connection*    m_DBcon;
    sql::Driver*        m_DBdriver;
    std::map<int, int*> m_UserMap;
    CTaskQueue*         m_workQue;
    CTaskQueue*         m_WorkProcessQue;
public:
    DBthread();
    ~DBthread();
    void Init(CTaskQueue* workque, CTaskQueue* workProcessQue);
    static void* Start(TVOID *pParam);
    void WorkRoutine();
    void Store();
};