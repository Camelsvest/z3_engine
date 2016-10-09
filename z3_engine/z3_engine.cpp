#include "z3_common.hpp"
#include "z3_engine.hpp"
#include "z3_iocp_obj.hpp"

#define DEFAULT_IOCP_TIMEOUT    10      // millseconds
#define MAX_IOCP_TIMEOUT        1000    // millseconds

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif


Engine::Engine(uint32_t nObjID)
        : Thread(nObjID)
        , m_hIOCP(NULL)
{
}

Engine::~Engine()
{
        assert(m_hIOCP == NULL);
        assert(m_lstPendingOvl.empty());
}

bool Engine::OnThreadStart(void)
{
        bool bOK;

        bOK = Thread::OnThreadStart();
        if (!bOK)
                return false;

        assert(m_hIOCP == NULL);
        m_hIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);               
        
        return (m_hIOCP != NULL);
}

void Engine::RunOnce()
{
        BOOL            bOK;
        DWORD           dwBytes, dwCompletionKey, dwTimeout;
        LPOVERLAPPED    pOvl;
        LPZ3OVL         pZ3Ovl, pTimeoutOvl;
        __int64         interval;  
        struct __timeb64 now, latest;

        /*TRACE_ENTER_FUNCTION;*/

        if (m_lstPendingOvl.empty())
        {
                pTimeoutOvl = NULL;
                dwTimeout = DEFAULT_IOCP_TIMEOUT; // millseconds
        }
        else
        {
                pTimeoutOvl = *m_lstPendingOvl.begin();
                latest = pTimeoutOvl->timeout;   // current time

                _ftime64_s(&now);
                
                interval = (latest.time * 1000 + latest.millitm) - (now.time * 1000 + now.millitm);
                if (interval >= DEFAULT_IOCP_TIMEOUT)
                {
                        dwTimeout = DEFAULT_IOCP_TIMEOUT;
                        pTimeoutOvl = NULL;
                }
                else
                {
                        if (interval > 0)
                                dwTimeout = interval;
                        else
                                dwTimeout = 0;
                }
        }

        assert(m_hIOCP);
        assert(dwTimeout < MAX_IOCP_TIMEOUT);       // 不能超过1秒
        bOK = ::GetQueuedCompletionStatus(m_hIOCP, &dwBytes, 
                        (PULONG_PTR)&dwCompletionKey, (LPOVERLAPPED *)&pOvl, dwTimeout);

        if (pOvl)
        {
                if (GET_EV_ID(pOvl) == EV_TIMEOUT)
                {
                        pZ3Ovl = Z3OVL_ADDR_FROM_TIMEOUTOVL(pOvl);                        
                        AddIntoPendingList(pZ3Ovl);

                        // Since now pZ3Ovl->timeout_ovl shall not be used
                }
                else
                {
                        pZ3Ovl = Z3OVL_ADDR_FROM_ACTOVL(pOvl);

                        // remove from pending list, then timeout shall be invalid
                        // activate event

                        RemoveFromPendingList(pZ3Ovl);
                        Dispatch(GET_EV_ID(pOvl), pZ3Ovl);
                }
        }
        else if (pTimeoutOvl)
        {
                assert(!bOK); // check point ?
                bOK = ::CancelIoEx(pTimeoutOvl->file_handle, ACT_OVL_ADDR(pTimeoutOvl));
                if (!bOK)
                        TRACE_ERROR("ErrorCode of CancelIoEx in %s:%d, : %lu\r\n", __FILE__, __LINE__, ::GetLastError());
        }

        /*TRACE_EXIT_FUNCTION;*/

        return;
}

void Engine::OnThreadStop(void)
{
        LPZ3OVL pZ3Ovl;
        IOCPObj *pObj;
        ev_id_t evID;
        void    *pData;
        
        assert(m_hIOCP);
        Z3_CLOSE_HANDLE(m_hIOCP);

        while (!m_lstPendingOvl.empty())
        {
                pZ3Ovl = m_lstPendingOvl.front();
                m_lstPendingOvl.pop_front();

                pObj = static_cast<IOCPObj *>(pZ3Ovl->data);

                pObj->Lock();
                pObj->FreeZ3Ovl(pZ3Ovl);
                pObj->Unlock();
        }

        pData = m_Queue.Pop(evID);
        while (pData)
        {
                pZ3Ovl = static_cast<LPZ3OVL>(pData);
                pObj = static_cast<IOCPObj *>(pZ3Ovl->data);
                pObj->Lock();
                pObj->FreeZ3Ovl(pZ3Ovl);
                pObj->Unlock();

                pData = m_Queue.Pop(evID);
        }

        Thread::OnThreadStop();
}

void Engine::AddIntoPendingList(LPZ3OVL pOvl)
{
        Z3OVL_LIST_ITERATOR itera;
        bool bAdded = false;

        for (itera = m_lstPendingOvl.begin(); itera != m_lstPendingOvl.end(); itera++)
        {
                if ((pOvl->timeout.time < (*itera)->timeout.time) || 
                        (pOvl->timeout.time == (*itera)->timeout.time && pOvl->timeout.millitm < (*itera)->timeout.millitm))
                {
                        m_lstPendingOvl.insert(itera, pOvl);
                        bAdded = true;
                        break;
                }
        }

        if (!bAdded)
                m_lstPendingOvl.push_back(pOvl);

        return;
}

void Engine::RemoveFromPendingList(LPZ3OVL pOvl)
{
        Z3OVL_LIST_ITERATOR itera;

#ifdef _DEBUG
        itera = find(m_lstPendingOvl.begin(), m_lstPendingOvl.end(), pOvl);
        assert(itera != m_lstPendingOvl.end());
#endif
        m_lstPendingOvl.remove(pOvl);

        return;
}

bool Engine::Dispatch(ev_id_t evID, LPZ3OVL pZ3Ovl)
{

        // 投递入异步队列
        m_Queue.Push(evID, pZ3Ovl);
        
        return m_Queue.Signal();
}