#include "DBth.h"

DBthread::DBthread()
{

}

void DBthread::Init(CTaskQueue* workque, CTaskQueue* workProcessQue)
{
    m_workQue = workProcessQue;
    m_WorkProcessQue = workque;
    m_DBdriver = get_driver_instance();
    m_DBcon = m_DBdriver->connect(URL, USER, PASS);

    sql::Statement* stmt = NULL;
    stmt = m_DBcon->createStatement();
    stmt->execute("USE " DATABASE);

    sql::ResultSet* res;
    res = stmt->executeQuery("SELECT * FROM Resource");

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
    int i = 0;
    while (true)
    {
        db->WorkRoutine();
        i++;
        if (i == 1000)
        {
            db->Store();
            i = 0;
        }
    }
}

void DBthread::Store()
{
    sql::PreparedStatement* stmt;
    stmt = m_DBcon->prepareStatement("UPDATE Resource SET Gold=(?), Wood=(?) WHERE UUID=(?)");
    std::map<int, int*>::iterator it = m_UserMap.begin();
    while (it != m_UserMap.end())
    {
        stmt->setInt(1, (it->second)[0]);
        stmt->setInt(2, (it->second)[1]);
        stmt->setInt(3, it->first);
        stmt->execute();
        it++;
    }
}

void DBthread::WorkRoutine()
{
    SessionWrapper* session = 0;
    m_workQue->WaitTillPop(session);

    Resources* node = (Resources*)session->ptr;

    sleep(5);

    if (node->serviceType == EN_SERVICE_TYPE_RESOURCE_GET)
    {
        for (int i = 0; i < node->num; i++)
        {
            (m_UserMap[node->nodes[i].UUID])[node->nodes[i].type] += node->nodes[i].num;
        }
    }
    else
    {
        for (int i = 0; i < node->num; i++)
        {
            (m_UserMap[node->nodes[i].UUID])[node->nodes[i].type] -= node->nodes[i].num;
        }
    }
    session->m_retCode = 0;
    session->m_sState = SEND_UNLOCK;
    m_WorkProcessQue->WaitTillPush(session);
    Store();
}