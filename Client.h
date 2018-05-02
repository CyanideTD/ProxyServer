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

	void 	ParsePackage(SessionWrapper* session, ResourceNode* node, ProxyData* data);
	void 	ParseReqPackage(SessionWrapper* session, ResourceNode* node, ProxyData* data);
	void 	ParseTextPackage(SessionWrapper* session, ResourceNode* node, ProxyData* data);
	void 	ParseResPackage(SessionWrapper* session, ResourceNode* node, ProxyData* data);
	void	ParseBinaryReqPackage(SessionWrapper* session, ResourceNode* node, ProxyData* data);

	void  	SendToServer(SessionWrapper* session, ResourceNode* node, ProxyData* data);

	void 	ProcessPackage(SessionWrapper* session, ResourceNode* node, ProxyData* data);

	void 	SendPackage(SessionWrapper* session, ProxyData* data);
	void 	SendTextPackage(SessionWrapper* session, ProxyData* data);

	void 	DoSomeThing(SessionWrapper* session, ResourceNode* node, ProxyData* data);
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