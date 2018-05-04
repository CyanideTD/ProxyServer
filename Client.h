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
	TVOID   WorkRoutine();

    void     init(CTaskQueue* taskQueue, CTaskQueue* recvQue, ILongConn* send, ILongConn* httpsend, ILongConn* lock, LongConnHandle lockserver, CTaskQueue* dbque);

	void 	ParsePackage(SessionWrapper* session, ProxyData* data, bool* bIsContinue);
	void 	ParseReqPackage(SessionWrapper* session, ProxyData* data, bool* bIsContinue);
	void 	ParseTextPackage(SessionWrapper* session, ProxyData* data, bool* bIsContinue);
	void 	ParseResPackage(SessionWrapper* session, ProxyData* data, bool* bIsContinue);
	void	ParseBinaryReqPackage(SessionWrapper* session, ProxyData* data, bool* bIsContinue);

	void  	SendToServer(SessionWrapper* session, ProxyData* data, bool* bIsContinue);

	void 	ProcessPackage(SessionWrapper* session, ProxyData* data);

	void 	SendPackage(SessionWrapper* session, ProxyData* data);
	void 	SendTextPackage(SessionWrapper* session, ProxyData* data);

	void 	EncounterError(SessionWrapper* session);
private:
    CBaseProtocolPack* pack;
    CBaseProtocolUnpack* unPack;
	CTaskQueue*			 m_dWorkQueue;
	LongConnHandle		 lock_server;
	ILongConn*			 m_IBinaryRecvConn;
	CTaskQueue*          m_ReceQueue;
	ILongConn*			 m_IHttpRecvConn;
	ILongConn*			 m_ILockConn;

	CTaskQueue*		     m_dbQue;

};

#endif