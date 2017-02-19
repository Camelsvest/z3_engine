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
        LPZ3_EV_OVL     pZ3Ovl; // wait whether it expires
        __int64         interval;  
        struct __timeb64 now, latest;

        /*TRACE_ENTER_FUNCTION;*/

        if (m_lstPendingOvl.empty())
        {
                pZ3Ovl = NULL;
                dwTimeout = DEFAULT_IOCP_TIMEOUT; // millseconds
        }
        else
        {
                pZ3Ovl = *m_lstPendingOvl.begin();
                latest = pZ3Ovl->timeout;   // current time

                _ftime64_s(&now);
                
                interval = (latest.time * 1000 + latest.millitm) - (now.time * 1000 + now.millitm);
                if (interval >= DEFAULT_IOCP_TIMEOUT)
                {
                        dwTimeout = DEFAULT_IOCP_TIMEOUT;
                        pZ3Ovl = NULL;
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
                switch (GET_EV_ID(pOvl))
                {
                case EV_OP_ADD:
                        AddIntoPendingList((LPZ3_EV_OVL)GET_EV_DATA(pOvl));
                        z3_free((LPZ3_EV_OVL)pOvl);
                        break;
                case EV_OP_REMOVE:
                        RemoveFromPendingList((LPZ3_EV_OVL)GET_EV_DATA(pOvl));
                        z3_free((LPZ3_EV_OVL)pOvl);
                        break;
                default:
                        RemoveFromPendingList((LPZ3_EV_OVL)pOvl);
                        Dispatch(GET_EV_ID(pOvl), (LPZ3_EV_OVL)pOvl, false);
                        break;
                }
        }
        else if (pZ3Ovl)
        {
                assert(!bOK); // check point ?

                Dispatch(GET_EV_ID(pZ3Ovl), pZ3Ovl, true);
                //bOK = ::CancelIoEx(pTimeoutOvl->file_handle, ACT_OVL_ADDR(pTimeoutOvl));
                //if (!bOK)
                //        TRACE_ERROR("ErrorCode of CancelIoEx in %s:%d, : %lu\r\n", __FILE__, __LINE__, ::GetLastError());

        }

        /*TRACE_EXIT_FUNCTION;*/

        return;
}

void Engine::OnThreadStop(void)
{
        LPZ3_EV_OVL pZ3Ovl;
        ev_id_t evID;
        void    *pData;
        
        assert(m_hIOCP);
        Z3_CLOSE_HANDLE(m_hIOCP);

        while (!m_lstPendingOvl.empty())
        {
                pZ3Ovl = m_lstPendingOvl.front();
                m_lstPendingOvl.pop_front();

                switch (GET_EV_ID(pZ3Ovl))
                {
                case EV_OP_ADD:
                case EV_OP_REMOVE:
                        FreeZ3Ovl((LPZ3_EV_OVL)GET_EV_DATA(pZ3Ovl));
                        FreeZ3Ovl(pZ3Ovl);
                        break;
                default:
                        FreeZ3Ovl(pZ3Ovl);
                        break;
                }
        }

        pData = m_Queue.Pop(evID);
        while (pData)
        {
                pZ3Ovl = static_cast<LPZ3_EV_OVL>(pData);              
                FreeZ3Ovl(pZ3Ovl);
                
                pData = m_Queue.Pop(evID);
        }

        Thread::OnThreadStop();
}

void Engine::AddIntoPendingList(LPZ3_EV_OVL pOvl)
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

void Engine::RemoveFromPendingList(LPZ3_EV_OVL pOvl)
{
        Z3OVL_LIST_ITERATOR itera;

#ifdef _DEBUG
        itera = find(m_lstPendingOvl.begin(), m_lstPendingOvl.end(), pOvl);
        assert(itera != m_lstPendingOvl.end());
#endif
        m_lstPendingOvl.remove(pOvl);

        return;
}

void Engine::RemoveFromPendingList(uint32_t handle)
{
        LPZ3_EV_OVL pZ3Ovl;

        Z3OVL_LIST_ITERATOR itera;

        for (itera = m_lstPendingOvl.begin(); itera != m_lstPendingOvl.end(); itera++)
        {
                pZ3Ovl = *itera;
                if (pZ3Ovl->ev_id == EV_TIMEOUT)
                {
                        if (pZ3Ovl->handle.timer_id == handle)
                                itera = m_lstPendingOvl.erase(itera);
                }
                else
                {
                        if (pZ3Ovl->handle.file_handle == (HANDLE)handle)
                                itera = m_lstPendingOvl.erase(itera);
                }
        }

        return;
}


bool Engine::Dispatch(ev_id_t evID, LPZ3_EV_OVL pZ3Ovl, bool bTimeout/* = false*/)
{
        // 投递入异步队列
        m_Queue.Push(evID, bTimeout, pZ3Ovl);
        
        return m_Queue.Signal();
}