#include "z3_common.hpp"
#include "z3_iocp_obj.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

IOCPObj::IOCPObj(HANDLE hIOCP, uint32_t nObjID)
        : AsyncObj(nObjID)
        , m_hIOCP(hIOCP)
{
        assert(m_hIOCP != NULL);
}

IOCPObj::~IOCPObj()
{
        LPZ3_EV_OVL pOvl;

        while(!m_lstIdle.empty())
        {
                pOvl = m_lstIdle.front();

                z3_free(pOvl);
                m_lstIdle.pop_front();
        }
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

        nError = AddEvent(pZ3Ovl);
        if (Z3_EOK != nError)
        {
                TRACE_ERROR("Failed to invoke function IOCPObj::AddEvent in connector object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                FreeZ3Ovl(pZ3Ovl);
                return nError;
        }

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

        nError = AddEvent(pZ3Ovl);
        if (Z3_EOK != nError)
        {
                TRACE_ERROR("Failed to AddEvent EV_READ in IOCPObj 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                FreeZ3Ovl(pZ3Ovl);
                return nError;
        }

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

        nError = AddEvent(pZ3Ovl);
        if (Z3_EOK != nError)
        {
                TRACE_ERROR("Failed to AddEvent EV_WRITE in IOCPObj 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                FreeZ3Ovl(pZ3Ovl);
                return nError;
        }

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

int IOCPObj::AddEvent(LPZ3_EV_OVL pZ3Ovl, bool bAdd /* = true*/)
{
        BOOL bOK;
        LPZ3_EV_OVL pOvl;

        pOvl = AllocZ3Ovl(NULL, bAdd ? EV_OP_ADD : EV_OP_REMOVE, INFINITE);
        
        if (pOvl == NULL)
                return Z3_ENOMEM;
        pOvl->data = pZ3Ovl;
        
        if (pOvl != NULL && m_hIOCP != NULL)
        {
                // 加入z3_engine 中的PENGDING列表，同时完成超时侦测
                // 触发线程将pZ3Ovl对象加入侦测链表
                bOK = ::PostQueuedCompletionStatus(m_hIOCP, 0, GetObjID(), (LPOVERLAPPED)pOvl);
                if (bOK)
                        return Z3_EOK;
        }

        return Z3_SYS_ERROR;
}

int IOCPObj::AddTimer(uint32_t nTimerID, uint32_t nMillseconds)
{
        //LPZ3_EV_OVL pZ3Ovl;

        //pZ3Ovl = AllocZ3Ovl((HANDLE)nTimerID, EV_TIMEOUT, nMillseconds);
        //if (pZ3Ovl == NULL)
        //        return Z3_ENOMEM;

        //return AddEvent(pZ3Ovl, true);


}

int IOCPObj::RemoveTimer(uint32_t nTimerID)
{
        LPZ3_EV_OVL pZ3Ovl;

        // timer is 0, it means timer shall be invalid, delete it
        pZ3Ovl = AllocZ3Ovl((HANDLE)nTimerID, EV_TIMEOUT, 0);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

        return AddEvent(pZ3Ovl, false);
}

int IOCPObj::Start(void)
{
        int             nError;
        LPZ3_EV_OVL     pZ3Ovl;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(NULL, EV_INSTANCE_START, INFINITE);
        assert(pZ3Ovl);

        nError = AddEvent(pZ3Ovl);
        if (nError != Z3_EOK)
        {
                FreeZ3Ovl(pZ3Ovl);
                return nError;                
        }

        // 投递一个“空”事件（实际上是随便投递一个事件），让IOCP启动，自动开始Dispatch，对象运行；
        /*bOK = ::PostQueuedCompletionStatus(m_hIOCP, 0, GetObjID(), ACT_OVL_ADDR(pZ3Ovl));
        if (!bOK)
        {
                TRACE_ERROR("Failed to invoke function PostQueuedCompletionStatus in connector object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                FreeZ3Ovl(pZ3Ovl);
                return Z3_SYS_ERROR;
        }*/

        return 0;        
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

        nError = AddEvent(pZ3Ovl);
        if (Z3_EOK != nError)
        {
                TRACE_ERROR("Failed to AddEvent EV_READ in IOCPObj 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                FreeZ3Ovl(pZ3Ovl);
                return nError;
        }

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
