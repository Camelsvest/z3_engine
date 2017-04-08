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

int IOCPObj::SocketAsyncConnect(SOCKET hSocket, SOCKADDR_IN *pTarget, uint32_t nMillseconds)
{
        int             nError;
        DWORD           dwBytes;
        LPFN_CONNECTEX  lpfnConnectEx;
        LPZ3_EV_OVL     pZ3Ovl;
        BOOL            bOK;
        GUID            GuidConnectEx = WSAID_CONNECTEX;

        if (hSocket == INVALID_SOCKET)
                return Z3_EINVAL;

        nError = ::WSAIoctl(hSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx),
                &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, 0, 0);
        assert(nError != SOCKET_ERROR);

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl((HANDLE)hSocket, EV_CONNECT, nMillseconds);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

        TRACE_DETAIL("Allocating Z3Ovl(0x%p) to connect host.\r\n", pZ3Ovl);

        /*AddIntoPendingList(pZ3Ovl);*/

        bOK = lpfnConnectEx(hSocket, (struct sockaddr *)pTarget, sizeof(SOCKADDR_IN),
                NULL, NULL, NULL, (LPOVERLAPPED)pZ3Ovl);
        if (!bOK)
        {
                nError = ::WSAGetLastError();
                if (nError != WSA_IO_PENDING)
                {
                        TRACE_ERROR("Failed to invoke function ConnectEx, error code is %d\r\n", nError);
                        return EWSABASE + nError;
                }
        }

        return Z3_EOK;
}

int IOCPObj::SocketAsyncTCPRead(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf)
{
        int     nError;
        LPZ3_EV_OVL pZ3Ovl;
        DWORD   dwRecvBytes, dwFlags;

        if (hSocket == INVALID_SOCKET || pwsaBuf == NULL)
                return Z3_EINVAL;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl((HANDLE)hSocket, EV_READ, SOCKET_READ_TIMEOUT);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

        /*AddIntoPendingList(pZ3Ovl);*/

        dwFlags = 0;
        nError = ::WSARecv(hSocket, pwsaBuf, 1, &dwRecvBytes, &dwFlags, (LPOVERLAPPED)pZ3Ovl, NULL);
        if (nError == SOCKET_ERROR)
        {
                nError = ::WSAGetLastError();
                if (nError != WSA_IO_PENDING)
                {
                        TRACE_ERROR("WSARecv failed, error code is %d\r\n", nError);
                        return EWSABASE + nError;
                }
        }

        return Z3_EOK;
}

int IOCPObj::SocketAsyncTCPWrite(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf)
{
        DWORD           dwSendBytes, dwFlags;
        int             nError;
        LPZ3_EV_OVL     pZ3Ovl;

        if (hSocket == INVALID_SOCKET || pwsaBuf == NULL || pwsaBuf->len <= 0)
                return Z3_EINVAL;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl((HANDLE)hSocket, EV_WRITE, SOCKET_WRITE_TIMEOUT);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

        /*AddIntoPendingList(pZ3Ovl);*/

        dwFlags = 0;
        nError = ::WSASend(hSocket, pwsaBuf, 1, &dwSendBytes, dwFlags, (LPOVERLAPPED)pZ3Ovl, NULL);
        if (SOCKET_ERROR == nError)
        {
                nError = ::WSAGetLastError();
                if (nError != WSA_IO_PENDING)
                {
                        TRACE_ERROR("WSASend failed, error code is %d\r\n", nError);
                        return EWSABASE + nError;
                }
        }

        return Z3_EOK;
}

int IOCPObj::SocketAsyncUDPRead(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf, SOCKADDR *pSockAddr, int *pnAddrSize)
{
        int     nError;
        LPZ3_EV_OVL pZ3Ovl;
        DWORD   dwRecvBytes, dwFlags;

        if (hSocket == INVALID_SOCKET || pwsaBuf == NULL)
                return Z3_EINVAL;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl((HANDLE)hSocket, EV_READ, SOCKET_READ_TIMEOUT);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

        /*AddIntoPendingList(pZ3Ovl);*/

        nError = ::WSARecvFrom(hSocket, pwsaBuf, 1, &dwRecvBytes, &dwFlags, pSockAddr,
                pnAddrSize, (LPOVERLAPPED)pZ3Ovl, NULL);
        if (nError == SOCKET_ERROR)
        {
                nError = ::WSAGetLastError();
                if (nError != WSA_IO_PENDING)
                {
                        TRACE_ERROR("WSARecv failed, error code is %d\r\n", nError);
                        return EWSABASE + nError;
                }
        }

        return Z3_EOK;
}

int IOCPObj::SocketAsyncUDPWrite(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf)
{
        assert(false);
        return Z3_EOK;
}

//int IOCPObj::AddTimer(uint32_t nTimerID, uint32_t nMillseconds)
//{
//        if (true == TimerObj::AddTimer(nTimerID, this, nMillseconds, false))
//                return Z3_EOK;
//        else
//                return Z3_ERROR;
//}

//int IOCPObj::RemoveTimer(uint32_t nTimerID)
//{
//        if (true == TimerObj::DeleteTimer(nTimerID))
//                return Z3_EOK;
//        else
//                return Z3_ERROR;
//}

LPZ3_EV_OVL IOCPObj::AllocZ3Ovl(HANDLE hFileHandle, ev_id_t evID, uint32_t millseconds)
{
        LPZ3_EV_OVL pOvl;

        pOvl = (LPZ3_EV_OVL)z3_calloc(1, sizeof(Z3_EV_OVL));
        if (!pOvl)
                return NULL;

        if (millseconds > 0  && millseconds < INFINITE)
        {
                if (!AddTimer(&pOvl->timer, pOvl, millseconds))
                {
                        z3_free(pOvl);
                        return NULL;
                }
        }
        else
                pOvl->timer = INVALID_HANDLE_VALUE;

        pOvl->iocp_handle = hFileHandle;

        pOvl->ev_id = evID;
        pOvl->data = this;

        return pOvl;
}

void IOCPObj::FreeZ3Ovl(LPZ3_EV_OVL pOvl)
{
        z3_free(pOvl);
}

int IOCPObj::Start(void)
{
        BOOL            bOK;
        LPZ3_EV_OVL     pZ3Ovl;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(NULL, EV_INSTANCE_START, 0);
        assert(pZ3Ovl);

        TRACE_DETAIL("Allocating Z3Ovl(0x%p) to start IOCP object.\r\n", pZ3Ovl);

        // 投递一个“空”事件（实际上是随便投递一个事件），让IOCP启动，自动开始Dispatch，对象运行；
        bOK = ::PostQueuedCompletionStatus(m_hIOCP, 0, GetObjID(), ACT_OVL_ADDR(pZ3Ovl));
        if (!bOK)
        {
                TRACE_ERROR("Failed to invoke function PostQueuedCompletionStatus in connector object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                FreeZ3Ovl(pZ3Ovl);
                return Z3_SYS_ERROR;
        }

        return 0;        
}


void IOCPObj::AddIntoPendingList(LPZ3_EV_OVL pOvl)
{
        Lock();
        m_lstPendingZ3Ovl.push_front(pOvl);
        Unlock();
}

void IOCPObj::OnTimer(void *pData)
{
        LPZ3_EV_OVL     pZ3Ovl;

	pZ3Ovl = static_cast<LPZ3_EV_OVL>(pData);
        assert(pZ3Ovl != NULL);
	assert(pZ3Ovl->iocp_handle != INVALID_HANDLE_VALUE);

        /*
  	 * 两种情况
	 * 1. 成功了，IO被取消，Z3_ENGINE中响应IO，并判断处理
	 * 2. 失败了，意味着IO正在进行中，无法取消，显然Z3_ENGINE可以继续处理IO事件
	 *
	 * 我们保障TIMER中的操作时间尽量短，大部分工作在Z3_ENGINE中触发完成
	 */
	::CancelIoEx(pZ3Ovl->iocp_handle, (LPOVERLAPPED)pZ3Ovl);

	return;
}