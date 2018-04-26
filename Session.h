#ifndef _SESSION_H_
#define _SESSION_H_

#include "std_header.h"
#include "svrpublib/ILongConn.h"
#include "queue_t.h"

typedef enum _State
{
    TO_LOCK,
    SEND_BACK,
    UNKNOW
}TaskStake;

class SessionWrapper
{
public:
    LongConnHandle m_stHandle;
    TUINT32        m_udwClientSeq;
    TUINT32        m_GlobalSeq;
    TUCHAR         m_szData[10 << 10];
    TUINT32        m_udwBufLen;
    bool           m_bIsBinaryData;
    TaskStake      m_sState;


    TVOID Init()
    {
        Reset();
    }

    TVOID Reset()
    {
        m_udwClientSeq = 0;
        m_GlobalSeq = 0;
        m_udwBufLen = 0;
        m_sState = UNKNOW;
        memset(m_szData, 0, sizeof(m_szData));
    }

};

typedef CQueue<SessionWrapper*>     CTaskQueue;

#endif