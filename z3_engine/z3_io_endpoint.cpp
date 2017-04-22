#include "z3_common.hpp"
#include "z3_io_endpoint.hpp"

using namespace Z3;

IOEndpoint::IOEndpoint(HANDLE hIOCP, uint32_t nObjID)
        : IOCPObj(hIOCP, nObjID)
        , m_hFileHandle(INVALID_HANDLE_VALUE)
{

}

IOEndpoint::~IOEndpoint()
{

}

void IOEndpoint::SetFileHandle(HANDLE hFileHandle)
{
        //Lock();

        assert(m_hFileHandle == INVALID_HANDLE_VALUE);

        m_hFileHandle = hFileHandle;

        //Unlock();
}

HANDLE IOEndpoint::GetFileHandle()
{
        return m_hFileHandle;
}

LPZ3_EV_OVL IOEndpoint::AllocZ3Ovl(ev_id_t evID, uint32_t millseconds)
{
        LPZ3_EV_OVL pOvl;

        pOvl = (LPZ3_EV_OVL)z3_calloc(1, sizeof(Z3_EV_OVL));
        if (!pOvl)
                return NULL;

        if (millseconds > 0 && millseconds < INFINITE)
        {
                if (!AddTimer(&pOvl->timer, pOvl, millseconds))
                {
                        z3_free(pOvl);
                        return NULL;
                }
        }
        else
                pOvl->timer = INVALID_HANDLE_VALUE;

//        assert(m_hFileHandle != INVALID_HANDLE_VALUE);
        pOvl->iocp_handle = m_hFileHandle;

        pOvl->ev_id = evID;
        pOvl->data = this;

        return pOvl;
}

void IOEndpoint::FreeZ3Ovl(LPZ3_EV_OVL pOvl)
{
        z3_free(pOvl);
}

int IOEndpoint::CancelIO(LPZ3_EV_OVL pOvl)
{
        BOOL bOK;

        assert(pOvl != NULL);
        assert(pOvl->iocp_handle != INVALID_HANDLE_VALUE);

        /*
        * 两种情况
        * 1. 成功了，IO被取消，Z3_ENGINE中响应IO，并判断处理
        * 2. 失败了，意味着IO正在进行中，无法取消，显然Z3_ENGINE可以继续处理IO事件
        *
        * 我们保障TIMER中的操作时间尽量短，大部分工作在Z3_ENGINE中触发完成
        */
        bOK = ::CancelIoEx(pOvl->iocp_handle, &(pOvl->ovl));

        return bOK ? Z3_EOK : Z3_SYS_ERROR;
}

int IOEndpoint::CancelTimer(LPZ3_EV_OVL pOvl)
{
        int     nRet;

        assert(pOvl != NULL);
        assert(pOvl->timer != INVALID_HANDLE_VALUE);

        if (TRUE == DeleteTimer(pOvl->timer))
        {
                nRet = Z3_EOK;
                pOvl->timer = INVALID_HANDLE_VALUE; // reset it now
        }
        else
                nRet = Z3_SYS_ERROR;

        return nRet;
}

void IOEndpoint::OnTimer(void *pData)
{
        LPZ3_EV_OVL     pZ3Ovl;

        assert(pData != NULL);
        pZ3Ovl = static_cast<LPZ3_EV_OVL>(pData);
        
        CancelIO(pZ3Ovl);

        return;
}

int IOEndpoint::Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes)
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
                nResult = OnEvCompleted(evID, nErrorCode, nBytes);
                break;
        }

        return nResult;
}

