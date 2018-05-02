#include "DBth.h"

void DBthread::Init(CTaskQueue* workque, CTaskQueue* workProcessQue)
{
    m_workQue = workque;
    m_WorkProcessQue = workProcessQue;
    m_DBdriver = get_driver_instance();
    m_DBcon = m_DBdriver->connect(URL, USER, PASS);

    sql::Statement* stmt = NULL;
    stmt = m_DBcon->createStatement();
    stmt->execute("USE " DATABASE);

    sql::ResultSet* res;
    res = stmt->executeQuery("SELECT * FROM Resources");

    while (res->next())
    {
        int id = res->getInt("UUID");
        int* resource = new int[2];
        resource[0] = res->getInt("Gold");
        resource[1] = res->getInt("Wood");
        m_UserMap[id] = resource;
    }

    delete stmt;
    delete res;
}

void* DBthread::Start(TVOID *pParam)
{
    DBthread* db = (DBthread*) pParam;
    while (true)
    {
        db->WorkRoutine();
    }
}

void DBthread::WorkRoutine()
{
    SessionWrapper* session = 0;
    m_workQue->WaitTillPop(session);

    ResourceNode* node = (ResourceNode*)session->ptr;

    (m_UserMap[node->UUID])[node->type] += node->num;
    session->m_sState = SEND_UNLOCK;
    m_WorkProcessQue->WaitTillPush(session);
}