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
    TUCHAR*        m_szData;
    TUINT32        m_udwBufLen;
    bool           m_bIsBinaryData;
    TaskStake      m_sState;


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
    }

};

typedef CQueue<SessionWrapper*>     CTaskQueue;

#endif