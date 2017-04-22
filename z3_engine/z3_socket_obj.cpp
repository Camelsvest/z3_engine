#include "z3_common.hpp"
#include "z3_socket_obj.hpp"

using namespace Z3;

SocketObj::SocketObj(HANDLE hIOCP, uint32_t nObjID)
        : IOEndpoint(hIOCP, nObjID)
{

}

SocketObj::~SocketObj()
{

}

int SocketObj::AsyncConnect(SOCKET hSocket, SOCKADDR_IN *pTarget, uint32_t nMillseconds)
{
        int             nError;
        DWORD           dwBytes;
        LPFN_CONNECTEX  lpfnConnectEx;
        LPZ3_EV_OVL     pZ3Ovl;
        BOOL            bOK;
        GUID            GuidConnectEx = WSAID_CONNECTEX;

        if (hSocket == INVALID_SOCKET)
                return Z3_EINVAL;

        SetFileHandle((HANDLE)hSocket);

        nError = ::WSAIoctl(hSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx),
                &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, 0, 0);
        assert(nError != SOCKET_ERROR);

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(EV_CONNECT, nMillseconds);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

        TRACE_DETAIL("Allocating Z3Ovl(0x%p) to connect host.\r\n", pZ3Ovl);

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

int SocketObj::AsyncTCPRead(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf)
{
        int     nError;
        LPZ3_EV_OVL pZ3Ovl;
        DWORD   dwRecvBytes, dwFlags;

        if (hSocket == INVALID_SOCKET || pwsaBuf == NULL)
                return Z3_EINVAL;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(EV_READ, SOCKET_READ_TIMEOUT);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

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

int SocketObj::AsyncTCPWrite(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf)
{
        DWORD           dwSendBytes, dwFlags;
        int             nError;
        LPZ3_EV_OVL     pZ3Ovl;

        if (hSocket == INVALID_SOCKET || pwsaBuf == NULL || pwsaBuf->len <= 0)
                return Z3_EINVAL;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(EV_WRITE, SOCKET_WRITE_TIMEOUT);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

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

int SocketObj::AsyncUDPRead(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf, SOCKADDR *pSockAddr, int *pnAddrSize)
{
        int     nError;
        LPZ3_EV_OVL pZ3Ovl;
        DWORD   dwRecvBytes, dwFlags;

        if (hSocket == INVALID_SOCKET || pwsaBuf == NULL)
                return Z3_EINVAL;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(EV_READ, SOCKET_READ_TIMEOUT);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

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

int SocketObj::AsyncUDPWrite(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf)
{
        assert(false);
        return Z3_EOK;
}
