#include "z3_common.hpp"
#include "z3_iocp_obj.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

IOCPObj::IOCPObj(HANDLE hIOCP, uint32_t nObjID)
        : TimerObj(nObjID)
        , m_hIOCP(hIOCP)
        , m_pNotifyQueue(NULL)
{
        assert(m_hIOCP != NULL);
}

IOCPObj::~IOCPObj()
{
        Z3_OBJ_RELEASE(m_pNotifyQueue);
}


int IOCPObj::PostCompletionStatus(LPOVERLAPPED pOvl)
{
        BOOL bOK;

        bOK = ::PostQueuedCompletionStatus(m_hIOCP, 0, GetObjID(), pOvl);
        if (!bOK)
        {
                TRACE_ERROR("Failed to invoke function PostQueuedCompletionStatus in IOCP object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                return Z3_SYS_ERROR;
        }

        return Z3_EOK;
}

void IOCPObj::Notify(ev_id_t evID, uint32_t nErrroCode)
{
        Z3EV_NOTIFY_ITEM item;

        TRACE_ENTER_FUNCTION;

        item.id = evID;
        item.nErrorCode = nErrroCode;
        item.data = this;

        if (m_pNotifyQueue)
        {
                Z3_OBJ_ADDREF(this);
                m_pNotifyQueue->Push(item);

                m_pNotifyQueue->Signal();
        }                

        TRACE_EXIT_FUNCTION;
}

int IOCPObj::Start(NOTIFY_QUEUE_T *pNotifyQueue)
{
        int             iRet;
        LPZ3_EV_OVL     pZ3Ovl;
     
        m_pNotifyQueue = pNotifyQueue;
        Z3_OBJ_ADDREF(m_pNotifyQueue);

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(EV_INSTANCE_START, 0);
        assert(pZ3Ovl);

        TRACE_DETAIL("Allocating Z3Ovl(0x%p) to start IOCP object.\r\n", pZ3Ovl);

        // 投递一个“EV_INSTANCE_START”事件，让IOCP启动，自动开始Dispatch，对象运行；
        iRet = PostCompletionStatus(ACT_OVL_ADDR(pZ3Ovl));
        if (iRet != Z3_EOK)
        {
                FreeZ3Ovl(pZ3Ovl);
                TRACE_DETAIL("Free Z3Ovl(0x%p) because failing to post completion status.\r\n", pZ3Ovl);
        }

        return iRet;
}

int IOCPObj::Stop(void)
{
        int             iRet;
        LPZ3_EV_OVL     pZ3Ovl;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(EV_INSTANCE_STOP, 0);
        assert(pZ3Ovl);

        TRACE_DETAIL("Allocating Z3Ovl(0x%p) to start IOCP object.\r\n", pZ3Ovl);

        // 投递一个“EV_INSTANCE_STOP”事件，让IOCP启动，自动开始Dispatch，对象运行；
        iRet = PostCompletionStatus(ACT_OVL_ADDR(pZ3Ovl));
        if (iRet != Z3_EOK)
        {
                FreeZ3Ovl(pZ3Ovl);
                TRACE_DETAIL("Free Z3Ovl(0x%p) because failing to post completion status.\r\n", pZ3Ovl);
        }

        return iRet;
}

int IOCPObj::Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes)
{
        int nResult = Z3_EOK;

        switch (evID)
        {
        case EV_INSTANCE_START:
                nResult = OnStart();
                break;

        case EV_INSTANCE_STOP:
                nResult = OnStop();
                break;

        default:
                TRACE_WARN("Unhandled event %u, you should process it on derived class.\r\n");
                break;
        }

        return nResult;
}

int IOCPObj::OnStart()
{
        m_bStarted = true;

        return Z3_EOK;
}

int IOCPObj::OnStop()
{
        assert(m_bStarted);
        m_bStarted = false;

        return Z3_EOK;
}

bool IOCPObj::IsStarted()
{
        bool bStarted;

        Lock();
        bStarted = m_bStarted;
        Unlock();

        return bStarted;
}

bool IOCPObj::IsStopped()
{
        bool bStopped;

        Lock();
        bStopped = !m_bStarted;
        Unlock();

        return bStopped;
}


