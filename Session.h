#ifndef _SESSION_H_
#define _SESSION_H_

#include "std_header.h"
#include "svrpublib/ILongConn.h"
#include "queue_t.h"


class SessionWrapper
{
public:
    LongConnHandle m_stHandle;
    TUINT32        m_udwClientSeq;
    TUINT32        m_GlobalSeq;
    TUCHAR         m_szReqBuf[10 << 10];
    TUINT32        m_udwReqBufLen;
    bool           m_bIsBinaryData;

    TVOID Init()
    {
        Reset();
    }

    TVOID Reset()
    {
        m_udwClientSeq = 0;
        m_GlobalSeq = 0;
        m_udwReqBufLen = 0;

    }

};

typedef CQueue<SessionWrapper*>     CTaskQueue;

#endif