#include "z3_common.hpp"
#include "z3_connector.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

Connector::Connector(HANDLE hIOCP, uint32_t nObjID)
        : IOCPObj(hIOCP, nObjID)
        , m_hSocket(INVALID_SOCKET)
        , m_ConnState(CONN_UNCONNECTED)
        , m_pRecvBuf(NULL)
        , m_nRecvBufSize(SOCKET_RECV_BUFSIZE)
        , m_pSendBuf(NULL)
        , m_nSendBufSize(SOCKET_SEND_BUFSIZE)
        , m_pszHost(NULL)
        , m_nPort(0)
{
        m_pSendBuf = (char *)z3_malloc(m_nSendBufSize);
        assert(m_pSendBuf);

        m_wsaSendBuf.buf = m_pSendBuf;
        m_wsaSendBuf.len = m_nSendBufSize;

        m_pRecvBuf = (char *)z3_malloc(m_nRecvBufSize);
        assert(m_pRecvBuf);

        m_wsaRecvBuf.buf = m_pRecvBuf;
        m_wsaRecvBuf.len = m_nRecvBufSize;
}

Connector::~Connector()
{
        Z3_FREE_POINTER(m_pSendBuf);
        Z3_FREE_POINTER(m_pRecvBuf);
        Z3_FREE_POINTER(m_pszHost);
        Z3_CLOSE_SOCKET(m_hSocket);

        //assert(m_ConnState == CONN_UNCONNECTED);
}

bool Connector::SetDestination(const char *pszHost, uint16_t nPort)
{
        size_t  nLength;

        if (pszHost == NULL || nPort <= 0)
                return false; 
        
        nLength = strlen(pszHost);
        if (nLength <= 0)
                return false;

        Z3_FREE_POINTER(m_pszHost);
        m_pszHost = (char *)z3_malloc(nLength + 1);
        if (!m_pszHost)
                return false;

        strcpy_s(m_pszHost, nLength+1, pszHost);
        m_nPort = nPort;

        return true;
}

int Connector::Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes, bool bExpired)
{
        int nResult = Z3_EOK;

        switch (evID)
        {
        case EV_INSTANCE_START:
                assert(m_ConnState == CONN_UNCONNECTED);
                if (m_pszHost && m_nPort > 0)
                        nResult = Connect(SOCKET_CONNECTING_TIMEOUT);
                break;

        case EV_INSTANCE_STOP:
                // ...
                break;

        default:
                nResult = OnEvCompleted(evID, nErrorCode, nBytes, bExpired);
                break;
        }

        //switch (m_ConnState)
        //{
        //case CONN_UNCONNECTED:
        //        if (m_pszHost && m_nPort > 0)
        //                nResult = Connect(SOCKET_CONNECTING_TIMEOUT);
        //        break;
        //case CONN_CONNECTING:
        //        assert(evID == EV_CONNECT);
        //        nResult = OnConnect(nErrorCode, bExpired);
        //        if (nResult == Z3_EOK && !bExpired)
        //                m_ConnState = CONN_CONNECTED;
        //        break;
        //case CONN_CONNECTED:
        //        nResult = OnEvCompleted(evID, nErrorCode, nBytes, bExpired);
        //        break;
        //default:
        //        break;
        //}

        return nResult;
}

int Connector::Connect(uint32_t nTimeout /*Millseconds*/)
{
        DWORD           dwBytes;
        BOOL            bOK;
        int             nError;
        HANDLE          hIOCP;
        SOCKADDR_IN     local;               
        PADDRINFOEX     pAddrInfoEx;
        SOCKADDR_IN     target;
        LPFN_CONNECTEX  lpfnConnectEx;
        GUID            GuidConnectEx   = WSAID_CONNECTEX;

        TRACE_ENTER_FUNCTION;

        pAddrInfoEx = InterpretDNS();
        if (pAddrInfoEx == NULL)
        {
                TRACE_ERROR("Cannot interpret DNS %s\r\n", m_pszHost);
                
                TRACE_EXIT_FUNCTION;
                return Z3_WSA_ERROR;
        }

        assert(m_hSocket == INVALID_SOCKET);
        m_hSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        assert (m_hSocket != INVALID_SOCKET);

        local.sin_addr.s_addr = ::htonl(INADDR_ANY);
        local.sin_family = AF_INET;
        local.sin_port = ::htons(0);

        if (SOCKET_ERROR == ::bind(m_hSocket, (struct sockaddr *)&local, sizeof(SOCKADDR_IN)))
        {
                TRACE_ERROR("Failed to bind socket in connector object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                ::FreeAddrInfoEx(pAddrInfoEx);

                TRACE_EXIT_FUNCTION;
                return Z3_WSA_ERROR;
        }

        // important, associates SOCKET with IOCP
        assert(m_hIOCP != NULL);
        hIOCP = ::CreateIoCompletionPort((HANDLE)m_hSocket, m_hIOCP, GetObjID(), 0);
        assert(hIOCP == m_hIOCP);

        memcpy(&target, pAddrInfoEx->ai_addr, sizeof(struct sockaddr));
        target.sin_port = ::htons(m_nPort);
        ::FreeAddrInfoEx(pAddrInfoEx);

        m_ConnState = CONN_CONNECTING;

        nError = SocketAsyncConnect(m_hSocket, &target, nTimeout);

        TRACE_EXIT_FUNCTION;
        return nError;
}

PADDRINFOEX Z3::Connector::InterpretDNS()
{
        int             nError;
        ADDRINFOEX      hints;
        PADDRINFOEX     pAddrInfoEx     = NULL;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family         = AF_INET;
        hints.ai_socktype       = SOCK_STREAM;
        hints.ai_protocol       = IPPROTO_TCP;

#if defined(_UNICODE)
        wchar_t *wszHost;

        nError = ::MultiByteToWideChar(CP_ACP, 0, m_pszHost, strlen(m_pszHost), NULL, 0);
        if (nError <= 0)
                return NULL;

        wszHost = (wchar_t *)z2_calloc(nError + 1, sizeof(wchar_t));
        nError = ::MultiByteToWideChar(CP_ACP, 0, m_pszHost, strlen(m_pszHost), wszHost, nError + 1);
        if (nError == 0)
        {
                Z2_FREE_POINTER(wszHost);
                TRACE_ERROR("Failed to convert MultiBytes character to UNICODE character, error code is %d\r\n",
                        ::GetLastError());
                        
                return NULL;
        }

        assert(m_pszHost);
        nError = ::GetAddrInfoEx(wszHost, 0, NS_DNS, NULL, &hints, 
                &pAddrInfoEx, NULL, NULL, NULL, NULL);
                        
        Z2_FREE_POINTER(wszHost);
#else
        assert(m_pszHost && (strlen(m_pszHost) > 0));
        nError = ::GetAddrInfoEx(m_pszHost, NULL, NS_DNS, NULL, &hints, 
                &pAddrInfoEx, NULL, NULL, NULL, NULL);
#endif
        if (nError != NO_ERROR)
        {
                TRACE_ERROR("Failed to invoke function GetAddrInfoEx, error code is %d\r\n", ::WSAGetLastError());
                return NULL;
        }

        return pAddrInfoEx;
}

int Connector::WriteMsg(Msg *pMsg)
{
        int             nError;
        unsigned int    nSize;
        char            *pBuf = NULL;

        assert(pMsg);
        if (!pMsg->ToString(&pBuf, &nSize))
                return Z3_EINVAL;

        assert(nSize > 0 && pBuf != NULL);
        nError = WriteMsg(pBuf, nSize);

        Z3_FREE_POINTER(pBuf);
        return nError;
}

int Connector::WriteMsg(const char *pBuf, uint32_t nSize)
{
        int             nError;

        TRACE_ENTER_FUNCTION;

        assert(nSize <= m_nSendBufSize);
        memmove(m_wsaSendBuf.buf, pBuf, nSize); // 可能存在区域重叠
        m_wsaSendBuf.len = nSize;

        assert(m_hSocket != INVALID_SOCKET);
        nError = SocketAsyncTCPWrite(m_hSocket, SOCKET_WRITE_TIMEOUT, &m_wsaSendBuf);

        TRACE_EXIT_FUNCTION;
        return nError;
}

int Connector::StartRead(uint32_t nTimeout /*Millseconds*/)
{
        int             nError;

        TRACE_ENTER_FUNCTION;

        assert(m_hSocket != INVALID_SOCKET);
        memset(m_wsaRecvBuf.buf, 0, m_wsaRecvBuf.len);
        nError = SocketAsyncTCPRead(m_hSocket, SOCKET_READ_TIMEOUT, &m_wsaRecvBuf);

        TRACE_EXIT_FUNCTION;

        return nError;
}

int Connector::OnEvCompleted(ev_id_t evID, uint32_t nStatusCode, uint32_t nBytes, bool bExpired)
{
        int nError = Z3_EOK;

        switch(evID)
        {
        case EV_READ:
                nError = OnEvRead(nStatusCode, nBytes, bExpired);
                break;
        case EV_WRITE:
                nError = OnEvWrite(nStatusCode, nBytes, bExpired);
                break;
        case EV_CONNECT:
                nError = OnConnect(nStatusCode, bExpired);
                break;
        default:
                assert(false);
                break;
        }

        return nError;
}

int Connector::OnConnect(uint32_t nStatusCode, bool bExpired)
{
        HRESULT hr;

        hr = HRESULT_FROM_NT(nStatusCode);
        if (SUCCEEDED(hr))
        {

        }

        return 0;
}

int Connector::OnEvRead(uint32_t nErrorCode, uint32_t nBytes, bool bExpired)
{
        ProtoParser     *pParser;
        Msg             *pMsg;
        uint32_t        nError = Z3_EOK;

        if (bExpired)
        {
                TRACE_WARN("Timer expired for operation \"READ\"\r\n");
                return Z3_EINTR;
        }

        if (nBytes <= 0)
        {
                // Peer closed 
                TRACE_ERROR("Connection has been closed by peer(dwBytes = %d), delete connector(0x%p) now\r\n", 
                        nBytes, this);                       

                // Cancel other event except EV_READ, because EV_READ has been received
                TRACE_WARN("Here need more processing, file %s, line %d", __FUNCTION__, __LINE__);

                return Z3_ERROR;
        }

        assert(nBytes < m_nRecvBufSize);
        *(m_pRecvBuf + nBytes) = '\0'; // string should be end with NULL

        TRACE_DEBUG("Received RTSP message(%lu bytes):\r\n", nBytes);
        TRACE_DUMP(LOG_DEBUG, m_pRecvBuf, nBytes);

        pParser = GetProtoParser();
        assert (pParser!= NULL);

        nError = pParser->Parse(m_pRecvBuf, nBytes, NULL, &pMsg);
        if (nError == 0)
        {                
                nError = Dispatch(pMsg, NULL);
        }

        return nError;
}

int Connector::OnEvWrite(uint32_t nErrorCode, uint32_t nBytes, bool bExpired)
{
        int     nError = Z3_EOK;

        if (bExpired)
        {
                TRACE_WARN("Timer expired for operation \"WRITE\"\r\n");
                return Z3_EINTR;
        }

        if (nErrorCode != 0)
        {
                TRACE_ERROR("Failed to send a message. Error code is %lu \r\n", nErrorCode);
                TRACE_DUMP(LOG_ERROR, m_wsaSendBuf.buf, m_wsaSendBuf.len);
                return Z3_EOK;
        }

        if (nBytes < m_wsaSendBuf.len)
        {
                TRACE_WARN("Cannot send a whole message, whole message size is %lu, send bytes is %u\r\n",
                        m_wsaSendBuf.len, nBytes);

                nError = WriteMsg(m_wsaSendBuf.buf + nBytes, m_wsaSendBuf.len - nBytes);
        }
        else
        {
                TRACE_DEBUG("Succeed to send a whole RTSP message\r\n");
                TRACE_DUMP(LOG_DEBUG, m_wsaSendBuf.buf, m_wsaSendBuf.len);
        }

        return nError;
}
