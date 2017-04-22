#include "z3_common.hpp"
#include "z3_iocp_obj.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

IOCPObj::IOCPObj(HANDLE hIOCP, uint32_t nObjID)
        : TimerObj(nObjID)
        , m_hIOCP(hIOCP)
{
        assert(m_hIOCP != NULL);
}

IOCPObj::~IOCPObj()
{
}


inline int IOCPObj::PostCompletionStatus(LPOVERLAPPED pOvl)
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

int IOCPObj::Start(void)
{
        int             iRet;
        LPZ3_EV_OVL     pZ3Ovl;

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
