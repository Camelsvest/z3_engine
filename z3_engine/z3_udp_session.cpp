#include "z3_common.hpp"
#include "z3_udp_session.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

UDPSession::UDPSession(HANDLE hIOCP, uint32_t nObjID)
        : SocketObj(hIOCP, nObjID)
        , m_nPort(0)
        , m_hIOCP(hIOCP)
        , m_pUdpRecvBuf(NULL)
        , m_nUdpRecvBufSize(SOCKET_RECV_BUFSIZE)
{
        m_addrFrom = { 0 };
        m_nAddrFromSize = 0;

        m_pUdpRecvBuf = (char *)z3_calloc(1, m_nUdpRecvBufSize);
        assert(m_pUdpRecvBuf);

        m_wsaRecvBuf.buf = m_pUdpRecvBuf;
        m_wsaRecvBuf.len = m_nUdpRecvBufSize;
}

UDPSession::~UDPSession()
{
        Z3_FREE_POINTER(m_pUdpRecvBuf);
}

int UDPSession::Init(uint16_t nPort)
{
        int             nError;
        SOCKET          hSocket;
        HANDLE          hIOCP;

        nError = SocketObj::Init(UDP_SOCK);
        if (nError != Z3_EOK)
        {
                TRACE_ERROR("Failed to create socket in UDPSession object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);
                return nError;
        }

        m_nPort = nPort;

        nError = Bind(nPort);
        if (nError != Z3_EOK)
                return nError;

        hSocket = GetSocket();
        assert(hSocket != INVALID_SOCKET);

        assert(m_hIOCP);
        hIOCP = ::CreateIoCompletionPort((HANDLE)hSocket, m_hIOCP, GetObjID(), 0);
        if (NULL == hIOCP)
        {
                TRACE_ERROR("Failed to create completion port in function %s, file %s, line %d\r\n",
                        __FUNCTION__, __FILE__, __LINE__);

                Close();
                return Z3_SYS_ERROR;
        }

        assert(hIOCP == m_hIOCP);

        return Z3_EOK;

}

int UDPSession::StartRead(uint32_t nTimeout /*Millseconds*/)
{
        int     nError;

        TRACE_ENTER_FUNCTION;

        memset(m_wsaRecvBuf.buf, 0, m_wsaRecvBuf.len);
        
        m_nAddrFromSize = sizeof(m_addrFrom);
        nError = AsyncUDPRead(nTimeout, &m_wsaRecvBuf, (LPSOCKADDR)&m_addrFrom, &m_nAddrFromSize);

        TRACE_EXIT_FUNCTION;

        return nError;
}

int UDPSession::Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes)
{
        int nError = Z3_EOK;

        switch (evID)
        {
        case EV_READ:
                nError = OnEvRead(nErrorCode, nBytes);
                break;
        case EV_WRITE:
                nError = OnEvWrite(nErrorCode, nBytes);
                break;
        default:
                assert(false);
                break;
        }


        return nError;
}

int UDPSession::OnEvRead(uint32_t nErrorCode, uint32_t nBytes)
{
        ProtoParser     *pParser;
        Msg             *pMsg;
        uint32_t        nError = Z3_EOK;

        if (nErrorCode != ERROR_SUCCESS)
        {
                TRACE_WARN("Failed for operation \"UDPSession::OnEvRead\", Error code: 0x%X.\r\n", nErrorCode);
                Close();

                return Z3_EINTR;
        }

        if (nBytes <= 0)
        {
                // Peer closed 
                TRACE_ERROR("UDPSession(0x%p) received %d bytes now.\r\n", this, nBytes);

                // Cancel other event except EV_READ, because EV_READ has been received
                TRACE_WARN("Here need more processing, file %s, line %d", __FUNCTION__, __LINE__);

                return Z3_ERROR;
        }

        // 在处理帧之前，先继续“读”，保障性能
        nError = StartRead(SOCKET_READ_TIMEOUT);
        assert(nError == 0);

        TRACE_DEBUG("Received UDP message(%lu bytes):\r\n", nBytes);
        TRACE_DUMP(LOG_DETAIL, m_pUdpRecvBuf, nBytes);

        pParser = GetProtoParser();
        assert (pParser!= NULL);

        nError = pParser->Parse(m_pUdpRecvBuf, nBytes, NULL, &pMsg);
        if (nError == 0)
                nError = Dispatch(pMsg, NULL);

        return nError;
}

int UDPSession::OnEvWrite(uint32_t nErrorCode, uint32_t nBytes)
{
        if (nErrorCode != ERROR_SUCCESS)
        {
                TRACE_WARN("Failed for operation \"UDPSession::OnEvWrite\", Error code: 0x%X.\r\n", nErrorCode);
                Close();

                return Z3_EINTR;
        }

        assert(false);
        return Z3_ERROR;
}
