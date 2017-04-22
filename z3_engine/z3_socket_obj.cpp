#include "z3_common.hpp"
#include "z3_socket_obj.hpp"

using namespace Z3;

SocketObj::SocketObj(HANDLE hIOCP, uint32_t nObjID)
        : IOEndpoint(hIOCP, nObjID)
        , m_hSocket(INVALID_SOCKET)
{

}

SocketObj::~SocketObj()
{
        assert(m_hSocket == INVALID_SOCKET);
}

int SocketObj::Init(SOCKET_TYPE type)
{
        assert(m_hSocket == INVALID_SOCKET);

        switch (type)
        {
        case TCP_SOCK:
                m_hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                break;
        case UDP_SOCK:
                m_hSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
                break;
        default:
                assert(false);
        }

        return (m_hSocket == INVALID_SOCKET) ? Z3_WSA_ERROR : Z3_EOK;
}

int SocketObj::Close(void)
{
        int nError;

        if (m_hSocket != INVALID_SOCKET)
        {
                nError = ::closesocket(m_hSocket);
                if (nError != 0)
                        return Z3_WSA_ERROR;

                m_hSocket = INVALID_SOCKET;
        }
        else
                TRACE_WARN("Duplicated operation, SocketObj(0x%X)\'s SOCKET HANDLE has been closed already.\r\n", this);

        return Z3_EOK;
}

int SocketObj::Bind(uint16_t nPort)
{
        SOCKADDR_IN     local;

        local.sin_addr.s_addr = ::htonl(INADDR_ANY);
        local.sin_family = AF_INET;
        local.sin_port = ::htons(nPort);

        assert(m_hSocket != INVALID_SOCKET);
        if (SOCKET_ERROR == ::bind(m_hSocket, (struct sockaddr *)&local, sizeof(SOCKADDR_IN)))
        {
                TRACE_ERROR("Failed to bind socket in SocketObj object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                return Z3_WSA_ERROR;
        }

        return Z3_EOK;
}

int SocketObj::AsyncConnect(SOCKADDR_IN *pTarget, uint32_t nMillseconds)
{
        int             nError;
        DWORD           dwBytes;
        LPFN_CONNECTEX  lpfnConnectEx;
        LPZ3_EV_OVL     pZ3Ovl;
        BOOL            bOK;
        GUID            GuidConnectEx = WSAID_CONNECTEX;

        if (m_hSocket == INVALID_SOCKET)
                return Z3_EINVAL;

        SetFileHandle((HANDLE)m_hSocket);

        nError = ::WSAIoctl(m_hSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx),
                &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, 0, 0);
        assert(nError != SOCKET_ERROR);

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(EV_CONNECT, nMillseconds);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

        TRACE_DETAIL("Allocating Z3Ovl(0x%p) to connect host.\r\n", pZ3Ovl);

        bOK = lpfnConnectEx(m_hSocket, (struct sockaddr *)pTarget, sizeof(SOCKADDR_IN),
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

int SocketObj::AsyncTCPRead(uint32_t nMillseconds, WSABUF *pwsaBuf)
{
        int     nError;
        LPZ3_EV_OVL pZ3Ovl;
        DWORD   dwRecvBytes, dwFlags;

        if (m_hSocket == INVALID_SOCKET || pwsaBuf == NULL)
                return Z3_EINVAL;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(EV_READ, SOCKET_READ_TIMEOUT);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

        dwFlags = 0;
        nError = ::WSARecv(m_hSocket, pwsaBuf, 1, &dwRecvBytes, &dwFlags, (LPOVERLAPPED)pZ3Ovl, NULL);
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

int SocketObj::AsyncTCPWrite(uint32_t nMillseconds, WSABUF *pwsaBuf)
{
        DWORD           dwSendBytes, dwFlags;
        int             nError;
        LPZ3_EV_OVL     pZ3Ovl;

        if (m_hSocket == INVALID_SOCKET || pwsaBuf == NULL || pwsaBuf->len <= 0)
                return Z3_EINVAL;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(EV_WRITE, SOCKET_WRITE_TIMEOUT);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

        dwFlags = 0;
        nError = ::WSASend(m_hSocket, pwsaBuf, 1, &dwSendBytes, dwFlags, (LPOVERLAPPED)pZ3Ovl, NULL);
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

int SocketObj::AsyncUDPRead(uint32_t nMillseconds, WSABUF *pwsaBuf, SOCKADDR *pSockAddr, int *pnAddrSize)
{
        int     nError;
        LPZ3_EV_OVL pZ3Ovl;
        DWORD   dwRecvBytes, dwFlags;

        if (m_hSocket == INVALID_SOCKET || pwsaBuf == NULL)
                return Z3_EINVAL;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(EV_READ, SOCKET_READ_TIMEOUT);
        if (pZ3Ovl == NULL)
                return Z3_ENOMEM;

        nError = ::WSARecvFrom(m_hSocket, pwsaBuf, 1, &dwRecvBytes, &dwFlags, pSockAddr,
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

int SocketObj::AsyncUDPWrite(uint32_t nMillseconds, WSABUF *pwsaBuf)
{
        assert(false);
        return Z3_EOK;
}
