#ifndef _CLIENT_H_
#define _CLIENT_H_
#include "Task.h"
#include "std_header.h"
#include "GlobalServ.h"
#include "queue_t.h"

#define hash_map __gnu_cxx::hash_map
extern int session;
extern long long tasks;
enum EDBPROXYKeySet
{
	//////////////////////////
	// GLOBAL KEY
	EN_GLOBAL_KEY__BEGIN = 0,
	EN_GLOBAL_KEY__RES_CODE,
	EN_GLOBAL_KEY__RES_COST_TIME,
	EN_GLOBAL_KEY__RES_TOTAL_RES_NUM,
	EN_GLOBAL_KEY__RES_CUR_RES_NUM,
	EN_GLOBAL_KEY__RES_BUF,

	// hu <-> hs
	EN_KEY_HU2HS__BEGIN = 5000,
	EN_KEY_HU2HS__REQ_NUM,
	EN_KEY_HU2HS__REQ_BUF,

	EN_KEY_HS2HU__BEGIN = 5100,
	EN_KEY_HS2HU__RES_DB_TYPE,
	EN_KEY_HS2HU__RES_TBL_TYPE,
	EN_KEY_HS2HU__RES_INDEX_OPENTYPE,
	EN_KEY_HS2HU__RES_COMMAND,

	// hu -> lock
	EN_KEY_HU2LOCK__BEGIN = 6000,
	EN_KEY_HU2LOCK__REQ_KEY_NUM,
	EN_KEY_HU2LOCK__REQ_KEY_LIST,
	EN_KEY_HU2LOCK__REQ_TIMEOUT_US,

	// lock -> hu
	EN_KEY_LOCK2HU__BEGIN = 6100,
};

enum RelationDataServiceType
{
	// hu <---> hs
	EN_HU2HS__BEGIN = 3000,
	EN_SERVICE_TYPE_HU2HS__SEARCH_REQ,
	EN_SERVICE_TYPE_HU2HS__UPDT_REQ,

	EN_HS2HU__BEGIN = 3100,
	EN_SERVICE_TYPE_HS2HU__SEARCH_RSP,
	EN_SERVICE_TYPE_HS2HU__UPDT_RSP,

	// hu <---> lock
	EN_HU2LOCK__BEGIN = 4000,
	EN_SERVICE_TYPE_HU2LOCK__GET_REQ,
	EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ,

	EN_LOCK2HU__BEGIN = 4100,
	EN_SERVICE_TYPE_LOCK2HU__GET_RSP,
	EN_SERVICE_TYPE_LOCK2HU__RELEASE_RSP,
};



class CWorkProcess
{
public :
    CWorkProcess();
    ~CWorkProcess();
    static void* Start(TVOID *pParam);
    TINT32   GetLock(bool* endSession);
    TINT32   ReleaseLock(bool* endSession);
    bool     connectTo();
    void     init(hash_map<int, int>* map, CQueue<Task>* lock, CQueue<Task>* unlock);
    void     EndSession();
private :
    int sockfd;
    CBaseProtocolPack* pack;
    CBaseProtocolUnpack* unPack;
    int seq;
    RelationDataServiceType serviceType;
    TUCHAR* buf;
	hash_map<int, int>*  m_LockFreqMap;
    CQueue<Task>*                   m_LockQue;
    CQueue<Task>*                   m_UnlockQue;
};

#endif