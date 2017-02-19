#include "z3_common.hpp"
#include "z3_async_queue.hpp"
#include "z3_iocp_obj.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif


AsyncQueue::AsyncQueue()
        : m_hEvent(NULL)
        , m_hMutex(NULL)
{
        m_hEvent = ::CreateEventA(NULL, TRUE, FALSE, NULL);
        assert(m_hEvent != NULL);

        m_hMutex = ::CreateMutexA(NULL, FALSE, NULL);
        assert(m_hMutex != NULL);
}

AsyncQueue::~AsyncQueue()
{
        ::CloseHandle(m_hMutex);
        ::CloseHandle(m_hEvent);
}


void AsyncQueue::Push(ev_id_t evID, bool bTimeout, void *pData)
{
        DWORD   dwResult;
        Z3EV    ev;

        ev.id = evID;
        ev.timeout = bTimeout;
        ev.data = pData;

        dwResult = ::WaitForSingleObject(m_hMutex, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);
        m_Queue.push(ev);
        ::ReleaseMutex(m_hMutex);        
}

void* AsyncQueue::Pop(ev_id_t &evID)
{
        DWORD   dwResult;
        Z3EV    ev;
        void    *pData;

        pData = NULL;

        dwResult = ::WaitForSingleObject(m_hMutex, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);
        if (!m_Queue.empty())
        {
                ev = m_Queue.front();
                m_Queue.pop();

                pData = ev.data;
                evID = ev.id;
        }
        ::ReleaseMutex(m_hMutex);

        return pData;
}

bool AsyncQueue::Signal(bool bOK)
{
        assert(m_hEvent);

        if (bOK)
                return (::SetEvent(m_hEvent) == TRUE);
        else
                return (::ResetEvent(m_hEvent) == TRUE);
}

bool AsyncQueue::WaitForEV(Z3EV &ev, uint32_t nTimeout/* millseconds*/)
{
        DWORD   dwResult;
        HANDLE  Handles[2];
        bool    bOk;

        assert(m_hMutex != NULL);
        assert(m_hEvent != NULL);

        Handles[0] = m_hMutex;
        Handles[1] = m_hEvent;
        bOk = false;

        dwResult = ::WaitForMultipleObjects(2, Handles, TRUE, nTimeout);        
        if (dwResult == WAIT_OBJECT_0)
        {
                if (!m_Queue.empty())
                {
                        ev = m_Queue.front();
                        m_Queue.pop();

                        bOk = true;
                }
        
                if (m_Queue.empty())
                        ::ResetEvent(m_hEvent);

                ::ReleaseMutex(m_hMutex);
        }

        return bOk;
}