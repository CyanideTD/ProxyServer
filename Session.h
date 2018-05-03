#ifndef _SESSION_H_
#define _SESSION_H_

#include "std_header.h"
#include "svrpublib/ILongConn.h"
#include "queue_t.h"

typedef enum _State
{
    GET_REQ = 0,
    GET_LOCK,
    GET_RES,
    HAVE_LOCK,
    SEND_UNLOCK,
    SEND_BACK,
    UNKNOW
}TaskStake;

class SessionWrapper
{
public:
    LongConnHandle m_stHandle;
    TUINT32        m_udwClientSeq;
    TUINT32        m_GlobalSeq;
    TUCHAR*        m_szData;
    TUINT32        m_udwBufLen;
    bool           m_bIsBinaryData;
    int            m_sState;
    TINT32         m_retCode;

    void*          ptr;


    TVOID Init()
    {
        m_szData = new TUCHAR[10240];
        Reset();
    }

    TVOID Reset()
    {
        memset(&m_stHandle, 0, sizeof(m_stHandle));
        m_udwClientSeq = 0;
        m_GlobalSeq = 0;
        m_udwBufLen = 0;
        m_sState = UNKNOW;
        memset(m_szData, 0, 10240);
        ptr = 0;
    }

};

typedef CQueue<SessionWrapper*>     CTaskQueue;

#endif