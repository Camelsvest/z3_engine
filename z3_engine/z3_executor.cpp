#include "z3_common.hpp"
#include "z3_executor.hpp"
#include "z3_iocp_obj.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

Executor::Executor(AsyncQueue *pQueue, uint32_t nObjID)
        : Thread(nObjID)
        , m_pQueue(pQueue)
{
        assert(m_pQueue != NULL);
}

Executor::~Executor()
{
}

bool Executor::OnThreadStart()
{
        return Thread::OnThreadStart();
}

void Executor::RunOnce()
{
        Z3EV ev;
        bool bSucceed;
        LPZ3_EV_OVL pZ3Ovl;
        IOCPObj *pObj;

        bSucceed = m_pQueue->WaitForEV(ev, 100 /*INFINITE*/);
        if (bSucceed)
        {
                assert(ev.id != EV_UNKNOWN);
                pZ3Ovl = static_cast<LPZ3_EV_OVL>(ev.data);             
                pObj = static_cast<IOCPObj *>(pZ3Ovl->data);
                
                pObj->Lock();                
                pObj->Run(ev.id, pZ3Ovl->ovl.Internal, pZ3Ovl->ovl.InternalHigh, ev.timeout);
                pObj->Unlock();

                pObj->FreeZ3Ovl(pZ3Ovl);                
        }
}

void Executor::OnThreadStop()
{
        Thread::OnThreadStop();
}