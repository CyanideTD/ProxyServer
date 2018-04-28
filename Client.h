#ifndef _CLIENT_H_
#define _CLIENT_H_
#include "Task.h"
#include "Session.h"
#include "std_header.h"
#include "GlobalServ.h"
#include "queue_t.h"

#define hash_map __gnu_cxx::hash_map
extern long long tasks;



class CWorkProcess
{
public :
    CWorkProcess();
    ~CWorkProcess();
    static void* Start(TVOID *pParam);
	TVOID*   WorkRoutine();

    void     init(CTaskQueue* taskQueue, CTaskQueue* recvQue, ILongConn* send, ILongConn* httpsend, ILongConn* lock, LongConnHandle lockserver, sql::Connection* con);

	void 	ParsePackage(SessionWrapper* session, ProxyData* data);
	void 	ProcessPackage(SessionWrapper* session, ProxyData* data);
	void 	SendPackage(SessionWrapper* session, ProxyData* data, bool* isContinue);
	void 	SendTextPackage(SessionWrapper* session, ProxyData* data);
	void 	DoSomeThing(SessionWrapper* session, ProxyData* data);
private:
    CBaseProtocolPack* pack;
    CBaseProtocolUnpack* unPack;
	CTaskQueue*			 m_dWorkQueue;
	LongConnHandle		 lock_server;
	ILongConn*			 m_IBinaryRecvConn;
	CTaskQueue*          m_ReceQueue;
	ILongConn*			 m_IHttpRecvConn;
	ILongConn*			 m_ILockConn;
	sql::Connection* 	 m_dbConn;

};

#endif