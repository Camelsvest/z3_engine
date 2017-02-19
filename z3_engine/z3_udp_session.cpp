#include "z3_common.hpp"
#include "z3_udp_session.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

UDPSession::UDPSession(HANDLE hIOCP, uint32_t nObjID)
        : IOCPObj(hIOCP, nObjID)
        , m_hSocket(INVALID_SOCKET)
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
        Z3_CLOSE_SOCKET(m_hSocket);
        Z3_FREE_POINTER(m_pUdpRecvBuf);
}

int UDPSession::Init(uint16_t nPort)
{
        SOCKADDR_IN     local;

        assert(m_hSocket == INVALID_SOCKET);
        m_hSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_hSocket == INVALID_SOCKET)
        {
                TRACE_ERROR("Failed to create socket in UDPSession object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);
                return Z3_WSA_ERROR;
        }

        m_nPort = nPort;

        local.sin_addr.s_addr   = ::htonl(INADDR_ANY);
        local.sin_family        = AF_INET;
        local.sin_port          = ::htons(m_nPort);

        if (SOCKET_ERROR == ::bind(m_hSocket, (struct sockaddr *)&local, sizeof(SOCKADDR_IN)))
        {
                TRACE_ERROR("Failed to bind socket in UDPSession object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                return Z3_WSA_ERROR;
        }

        assert(m_hIOCP);
        if (NULL == ::CreateIoCompletionPort((HANDLE)m_hSocket, m_hIOCP, GetObjID(), 0))
        {
                TRACE_ERROR("Failed to create completion port in function %s, file %s, line %d\r\n",
                        __FUNCTION__, __FILE__, __LINE__);

                Z3_CLOSE_SOCKET(m_hSocket);
                return Z3_SYS_ERROR;
        }

        return Z3_EOK;

}

int UDPSession::StartRead(uint32_t nTimeout /*Millseconds*/)
{
        int             nError;

        TRACE_ENTER_FUNCTION;

        assert(m_hSocket != INVALID_SOCKET);
        memset(m_wsaRecvBuf.buf, 0, m_wsaRecvBuf.len);
        
        m_nAddrFromSize = sizeof(m_addrFrom);
        nError = SocketAsyncUDPRead(m_hSocket, nTimeout, &m_wsaRecvBuf, (LPSOCKADDR)&m_addrFrom, &m_nAddrFromSize);

        TRACE_EXIT_FUNCTION;

        return nError;
}

int UDPSession::Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes, bool bExpired)
{
        int nError = Z3_EOK;

        switch (evID)
        {
        case EV_READ:
                nError = OnEvRead(nErrorCode, nBytes, bExpired);
                break;
        case EV_WRITE:
                nError = OnEvWrite(nErrorCode, nBytes, bExpired);
                break;
        default:
                assert(false);
                break;
        }


        return nError;
}

int UDPSession::OnEvRead(uint32_t nErrorCode, uint32_t nBytes, bool bExpired)
{
        ProtoParser     *pParser;
        Msg             *pMsg;
        uint32_t        nError = Z3_EOK;

        if (bExpired)
        {
                TRACE_WARN("UDP Session operation \"READ\" expired.\r\n");
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

int UDPSession::OnEvWrite(uint32_t nErrorCode, uint32_t nBytes, bool bExpired)
{
        if (bExpired)
        {
                TRACE_WARN("UDPSession operation \"WRITE\" expired.\r\n");
                return Z3_EINTR;
        }

        assert(false);
        return Z3_ERROR;
}
