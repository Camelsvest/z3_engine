#include "z3_common.hpp"
#include "z3_executor.hpp"
#include "z3_io_endpoint.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

Executor::Executor(EV_QUEUE_T *pQueue, uint32_t nObjID)
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
        Z3EV_ASYNCQUEUE_ITEM item;
        bool bSucceed;
        LPZ3_EV_OVL pZ3Ovl;
        IOEndpoint *pObj;

        //TRACE_ENTER_FUNCTION;

        bSucceed = m_pQueue->WaitForEV(item, 100 /*INFINITE*/);
        if (bSucceed)
        {
                assert(item.id != EV_UNKNOWN);
                pZ3Ovl = static_cast<LPZ3_EV_OVL>(item.data);
                pObj = static_cast<IOEndpoint *>(pZ3Ovl->data);
               
                pObj->Lock();                

                pObj->CancelTimer(pZ3Ovl);
                pObj->Run(item.id, pZ3Ovl->ovl.Internal, pZ3Ovl->ovl.InternalHigh);

                pObj->Unlock();

                pObj->FreeZ3Ovl(pZ3Ovl);                
        }

        //TRACE_EXIT_FUNCTION;
}

void Executor::OnThreadStop()
{
        TRACE_ENTER_FUNCTION;

        Thread::OnThreadStop();

        TRACE_EXIT_FUNCTION;
}