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
        ev_id_t evID;
        LPZ3OVL pZ3Ovl;
        void    *pData;
        IOCPObj *pObj;

        pData = m_pQueue->WaitForEV(evID);
        if (pData)
        {
                assert(evID != EV_UNKNOWN);
                pZ3Ovl = static_cast<LPZ3OVL>(pData);                
                pObj = static_cast<IOCPObj *>(pZ3Ovl->data);
                
                pObj->Lock();
                pObj->Run(evID, ACT_OVL_ADDR(pZ3Ovl)->Internal, ACT_OVL_ADDR(pZ3Ovl)->InternalHigh);
                pObj->FreeZ3Ovl(pZ3Ovl);
                pObj->Unlock();
        }
}

void Executor::OnThreadStop()
{
        Thread::OnThreadStop();
}