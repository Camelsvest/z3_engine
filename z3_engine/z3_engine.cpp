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
}

bool Engine::OnThreadStart(void)
{
        bool bOK;

        TRACE_ENTER_FUNCTION;

        bOK = Thread::OnThreadStart();
        if (!bOK)
                return false;

        assert(m_hIOCP == NULL);
        m_hIOCP = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);               
        
        TRACE_EXIT_FUNCTION;

        return (m_hIOCP != NULL);
}

void Engine::RunOnce()
{
        BOOL            bOK;
        DWORD           dwBytes, dwCompletionKey;
        LPZ3_EV_OVL     pZ3Ovl; // wait whether it expires
        LPOVERLAPPED    pOvl = NULL;

        //TRACE_ENTER_FUNCTION;

        assert(m_hIOCP);
        bOK = ::GetQueuedCompletionStatus(m_hIOCP, &dwBytes, 
                        (PULONG_PTR)&dwCompletionKey, (LPOVERLAPPED *)&pOvl, 100);

        if (pOvl)
        {
                TRACE_DETAIL("Ovl(0x%p) is available, dispatch...\r\n", pOvl);
                pZ3Ovl = (LPZ3_EV_OVL)pOvl;

                // Set Error Code 
                pZ3Ovl->ovl.Internal = bOK ? ERROR_SUCCESS : ::GetLastError();

                // Dispatch to ExecutorPool
                Dispatch(GET_EV_ID(pZ3Ovl), pZ3Ovl);
        }

        //TRACE_EXIT_FUNCTION;

        return;
}

void Engine::OnThreadStop(void)
{
        LPZ3_EV_OVL pZ3Ovl;
        Z3EV_ASYNCQUEUE_ITEM item;
        IOCPObj *pObj;
        bool bOK;
        
        assert(m_hIOCP);
        Z3_CLOSE_HANDLE(m_hIOCP);

        bOK = m_Queue.Pop(item);
        while (bOK)
        {
                if (item.data)
                {
                        pZ3Ovl = static_cast<LPZ3_EV_OVL>(item.data);
                        pObj = static_cast<IOCPObj *>(pZ3Ovl->data);
                        assert(pObj);

                        pObj->FreeZ3Ovl(pZ3Ovl);
                }
                
                bOK = m_Queue.Pop(item);
        }

        Thread::OnThreadStop();
}


bool Engine::Dispatch(ev_id_t evID, LPZ3_EV_OVL pZ3Ovl)
{
        Z3EV_ASYNCQUEUE_ITEM item;

        item.id = evID;
        item.data = pZ3Ovl;

        // 投递入异步队列
        m_Queue.Push(item);
        
        return m_Queue.Signal();
}