#include "z3_common.hpp"
#include "z3_rtsp_session.hpp"
#include "z3_rtsp_msg.hpp"
#include "z3_rtsp_proto_parser.hpp"
#include "z3_rtsp_error.hpp"
#include "z3_sdp.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif


RtspSession::RtspSession(SessionOwner *pOwner, HANDLE hIOCP, uint32_t nObjID)
        : Connector(hIOCP, nObjID)
        , m_pOwner(pOwner)
        , m_pszUrl(NULL)
        , m_methodPrevious(RTSP_INVALID)
        , m_nCSeqPrevious(0)
        , m_pszSessionIdPrevious(NULL)
        , m_pRtspMedia(NULL)
        , m_pParser(NULL)
        , m_nSessionState(RTSP_SESSION_INIT)
        , m_pszSessionId(NULL)
        , m_nTimeout(DEFAULT_RTSP_SESSION_TIMEOUT)
{
        Z3_OBJ_ADDREF(m_pOwner);
}

RtspSession::~RtspSession()
{
        Z3_FREE_POINTER(m_pszSessionId);
        Z3_DELETE_OBJ(m_pRtspMedia);
        Z3_OBJ_RELEASE(m_pParser);
        Z3_OBJ_RELEASE(m_pOwner);
        Z3_FREE_POINTER(m_pszUrl);
        Z3_FREE_POINTER(m_pszSessionIdPrevious);
}

int RtspSession::OnConnect(uint32_t nErrorCode)
{
        int nError = Z3_ERROR;

        if (nErrorCode != 0)
        {
                assert(m_pOwner);
                m_pOwner->OnNotify(STATE_CONNECT_TIMEOUT, this);

                return Z3_ERROR;
        }

        nError = SendOPTIONS();
        if (nError == Z3_EOK)
                nError = StartRead(SOCKET_READ_TIMEOUT);

        return nError;
}

int RtspSession::SendOPTIONS()
{
        int             nError;
        RtspMsg         *pRequest;
        const char      szCseq[] = "0";

        const char      szAgent[] = "RtspClient, which is developped by Z3Engine & RTSP util library\r\n";

        TRACE_ENTER_FUNCTION;

        pRequest = new RtspMsg;
        if (pRequest == NULL)
        {
                TRACE_EXIT_FUNCTION;
                return Z3_ENOMEM;
        }

        Z3_OBJ_ADDREF(pRequest);

        assert(m_pszUrl);
        nError = pRequest->InitRequest(RTSP_OPTIONS, m_pszUrl, strlen(m_pszUrl));
        if (nError == Z3_EOK)
        {
                nError = pRequest->AddHeader(RTSP_HDR_CSEQ, szCseq, sizeof(szCseq));
                assert(nError == Z3_EOK);

                nError = pRequest->AddHeader(RTSP_HDR_USER_AGENT, szAgent, sizeof(szAgent));
                assert(nError == Z3_EOK);

                nError = WriteMsg(pRequest);
                if (nError == 0)
                {
                        m_methodPrevious = RTSP_OPTIONS;
                        m_nCSeqPrevious = 0;
                }
        }
        else
                nError = Z3_ENOMEM;

        Z3_OBJ_RELEASE(pRequest);
        
        TRACE_EXIT_FUNCTION;

        return nError;
}

int RtspSession::SetRequestUrl(const char *pszUrl)
{
        char            *pszProtocol;
        char            *pszHost;
        char            *pos1, *pos2;
        uint16_t        nPort;
        int             nLength, nError;

        nLength = strlen(pszUrl);
        m_pszUrl = (char *)z3_malloc(nLength+1);
        if (m_pszUrl == NULL)
                return Z3_ENOMEM;

        strncpy_s(m_pszUrl, nLength + 1, pszUrl, nLength);

        // rtsp://hostname:port/resource
        pos1 = (char *)strchr(pszUrl, ':');
        if (pos1 == NULL)
        {
                TRACE_ERROR("Incorrect URL path, it must be \"rtsp://host:port/resource\"\r\n");
                return Z3_EINVAL;
        }

        nLength = pos1 - pszUrl + 1;
        pszProtocol = (char *)z3_malloc(nLength);
        strncpy_s(pszProtocol, nLength, pszUrl, nLength - 1);

        if (_stricmp(pszProtocol, "rtsp") != 0)
        {
                TRACE_ERROR("Incorrect protocol in Url path, it must be \"rtsp:\"\r\n");

                Z3_FREE_POINTER(pszProtocol);
                return Z3_EINVAL;
        }
        Z3_FREE_POINTER(pszProtocol);

        if ( *(pos1+1) != '/' && *(pos1+2) != '/')
        {
                TRACE_ERROR("protocal rtsp: must be followed by //, such as rtsp://...\r\n");
                return Z3_EINVAL;
        }

        pos2 = (char *)strchr(pos1 + 3, '/');
        
        nLength = pos2 - (pos1 + 3) + 1;
        pszHost = (char *)z3_malloc(nLength);
        strncpy_s(pszHost, nLength, pos1+3, nLength - 1);

        pos1 = (char *)strchr(pszHost, ':');
        if (pos1 == NULL)
                nPort = DEFAULT_RTSP_PORT;
        else
        {
                *pos1 = '\0';
                nPort = atoi(pos1 + 1);
        }
        
        if (SetDestination(pszHost, nPort))
                nError = Z3_EOK;
        else
                nError = Z3_ENOMEM;

        z3_free(pszHost);

        return nError;
}

ProtoParser* RtspSession::GetProtoParser()
{
        if (m_pParser == NULL)
        {
                m_pParser = new RtspProtoParser();
                Z3_OBJ_ADDREF(m_pParser);
        }
        
        return m_pParser;
}

int RtspSession::Dispatch(Msg *pMsg, void *pData)
{
        RtspMsg                 *pRtspMsg;
        RTSP_STATUS_CODE        code;
        RTSP_VERSION            version;
        char                    *pReason, *pCSeq, *pBuf;
        int                     result;
        uint32_t                nBufSize;

        assert(pMsg->ProtoID() == RTSP_PROTO_ID);
        pRtspMsg = dynamic_cast<RtspMsg *>(pMsg);

        if (pRtspMsg->GetMsgObj()->GetMsgType() != RTSP_MSG_RESPONSE)
        {
                TRACE_ERROR("Received invalid RTSP message\r\n");

                if (pRtspMsg->ToString(&pBuf, &nBufSize))
                {
                        TRACE_DUMP(LOG_ERROR, pBuf, nBufSize);
                        Z3_FREE_POINTER(pBuf);
                }
                return EINVAL;
        }
        
        result = pRtspMsg->GetResponse(&code, (const char **)&pReason, &version);
        if (result != Z3_EOK)
        {
                TRACE_ERROR("Failed to parse RTSP response\r\n");
                return EINVAL;
        }

        if (code != RTSP_STS_OK)
        {
                TRACE_INFO("response code(%d) != 200 OK\r\n", code);
                return 0;
        }

        result = pRtspMsg->GetHeader(RTSP_HDR_CSEQ, &pCSeq, 0);
        if (result != Z3_EOK)
        {
                TRACE_INFO("Server RTSP response, No 'CSeq:' field.\r\n");
                return 0;
        }

        if (atoi(pCSeq) != m_nCSeqPrevious)
        {
                TRACE_INFO("Unexpected RTSP response. CSeq does not match, previous CSeq = %d, received CSeq = %s\r\n",
                        m_nCSeqPrevious, pCSeq);
                return 0;
        }

        return InvokeStateMachine(m_methodPrevious, pRtspMsg, pData);
}

int RtspSession::InvokeStateMachine(RTSP_METHOD method, RtspMsg *pResponse, void *pData)
{
        int nError = Z3_ERROR;

        switch (method)
        {
        case RTSP_OPTIONS:
                nError = OnOptionsRespond(pResponse, pData);
                break;
        case RTSP_DESCRIBE:
                nError = OnDescribeRespond(pResponse, pData);
                break;
        case RTSP_SETUP:
                nError = OnSetupRespond(pResponse, pData);
                break;
        case RTSP_PLAY:
                break;
        default:
                assert(false);
                break;
        };

        if (nError == 0)
        {
                nError = StartRead(SOCKET_READ_TIMEOUT);
                assert(nError == 0);
        }

        return 0;
}

int RtspSession::OnOptionsRespond(RtspMsg *pResponse, void *pData)
{
        char    *pValue;
        int     result;
        int     nIndex, nError = Z3_RTSP_EUNSUPPORTED;

        nIndex = 0;
        while (true)
        {
                result = pResponse->GetHeader(RTSP_HDR_PUBLIC, &pValue, nIndex++);
                if (result != Z3_EOK)
                        break;

                if (_stricmp(pValue, "describe") == 0)
                {
                        nError = SendDESCRIBE(pResponse);
                        break;
                }
        }

        return nError;
}

int RtspSession::SendDESCRIBE(RtspMsg *pResponse)
{
        int             nError;
        int             result;
        RtspMsg         *pRequest;
        char            szCSeq[16];
        const char      szAccept[] = "application/sdp";

        pRequest = new RtspMsg;
        if (pRequest == NULL)
                return Z3_ENOMEM;
        
        Z3_OBJ_ADDREF(pRequest);

        assert(m_pszUrl);
        result = pRequest->InitRequest(RTSP_DESCRIBE, m_pszUrl, strlen(m_pszUrl));
        if (result == Z3_EOK)
        {
                sprintf_s(szCSeq, sizeof(szCSeq), "%d", ++m_nCSeqPrevious);
                result = pRequest->AddHeader(RTSP_HDR_CSEQ, szCSeq, sizeof(szCSeq));
                assert(result == Z3_EOK);

                result = pRequest->AddHeader(RTSP_HDR_ACCEPT, szAccept, sizeof(szAccept));
                assert(result == Z3_EOK);

                nError = WriteMsg(pRequest);
                if (nError == 0)
                {
                        m_methodPrevious = RTSP_DESCRIBE;
                }
        }
        else
                nError = Z3_ENOMEM;

        Z3_OBJ_RELEASE(pRequest);

        return nError;
}

int RtspSession::OnDescribeRespond(RtspMsg *pResponse, void *pData)
{
        int             result;
        SDPMessage      *pSDP;
        unsigned char   *pSDPString;
        char            *pContentBase;
        int             nBytes, nError;
        unsigned int    nTracks;
        RtspMedia       *pMedia;

        assert(m_nSessionState == RTSP_SESSION_INIT);
        assert(m_pRtspMedia == NULL);

        pSDPString = pResponse->GetBody(&nBytes);
        if (nBytes <= 0 || pSDPString == NULL)
        {
                TRACE_ERROR("No SDP context in DESCRIBE response\r\n");
                return Z3_EINVAL;
        }

        pSDP = new SDPMessage;
        if (pSDP == NULL)
        {
                TRACE_ERROR("Failed to allocate object SDPMessage, no memory now\r\n");
                return Z3_ENOMEM;
        }

        nError = pSDP->ParseString((char *)pSDPString, nBytes);
        if (nError != 0)
        {
                TRACE_ERROR("Failed to parse SDP string in DESCRIBE response\r\n");
                return nError;
        }

        nTracks = pSDP->GetMediaCount();
        if (nTracks < 1)
        {
                TRACE_ERROR("Media dosn't have stream!\r\n");
                Z3_DELETE_OBJ(pSDP);

                return Z3_RTSP_ESTREAMCOUNTS;
        }

        TRACE_DEBUG("Media has %d stream(s)!\r\n", nTracks);

        result = pResponse->GetHeader(RTSP_HDR_CONTENT_BASE, &pContentBase, 0);
        if (result != Z3_EOK)
        {
                TRACE_ERROR("No 'content-base' in DESCRIBE response!\r\n");
                Z3_DELETE_OBJ(pSDP);

                return result;
        }

        pMedia = new RtspMedia();
        if (pMedia == NULL)
                return Z3_ENOMEM;

        if (!pMedia->Init(pContentBase, nTracks, 0, pSDP))
        {
                Z3_DELETE_OBJ(pMedia);
                return Z3_ENOMEM;
        }

        m_pRtspMedia = pMedia;
        
        return SendSETUP();
}

int RtspSession::OnSetupRespond(RtspMsg *pResponse, void *pData)
{
        char            *pszSessionId, *pszTimeout, *pToken = NULL;
        int             result;
        unsigned int    nRequestTrack;

        assert(m_pRtspMedia);

        result = pResponse->GetHeader(RTSP_HDR_SESSION, &pszSessionId, 0);
        if (result != Z3_EOK)
        {
                TRACE_ERROR("Server SETUP response, No 'Session:' field.\r\n");
                return Z3_RTSP_ENOSESSIONID;
        }

        if (m_pszSessionId == NULL)
        {
                m_pszSessionId = z3_strdup(pszSessionId);

                strtok_s(m_pszSessionId, ";", &pToken);

                pszTimeout = strstr(m_pszSessionId, "timeout=");
                if (pszTimeout)
                {
                        sscanf_s(pszTimeout, "timeout=%d", &m_nTimeout);
                        if (m_nTimeout <= 0)
                                m_nTimeout = DEFAULT_RTSP_SESSION_TIMEOUT;
                }
        }
        else
        {
                if (strncmp(pszSessionId, m_pszSessionId, strlen(m_pszSessionId)))
                {
                        TRACE_ERROR("SETUP response changed 'Session:' value.\r\n");
                        return Z3_RTSP_ESESSIONID;
                }
        }
        
        nRequestTrack = m_pRtspMedia->GetRequestTrack();
        if (nRequestTrack >= m_pRtspMedia->GetTrackCount())
        {
                m_nSessionState = RTSP_SESSION_READY;
                return SendPLAY();
        }
        else
        {
                return SendSETUP();
        }

}

int RtspSession::SendSETUP()
{
        char            szBuf[256], *pValue, *pContentBase;
        SDPMessage      *pSDP;
        SDPMedia        *pSDPMedia;
        MediaSession    *pMediaSession;
        int             nLength, nError, nRequestTrack;
        RtspMsg         *pRequest;
        int             result;
        uint16_t        nRtpPort, nRtcpPort;

        assert(m_pRtspMedia);
        pSDP = m_pRtspMedia->GetSDPMessage();
        assert(pSDP);

        nRequestTrack = m_pRtspMedia->GetRequestTrack();
        pSDPMedia = pSDP->GetSDPMedia(nRequestTrack);
        assert(pSDPMedia);
        m_pRtspMedia->SetRequestTrack(++nRequestTrack);

        pValue = pSDPMedia->GetAttributeValue("control");
        assert(pValue);

        pContentBase = m_pRtspMedia->GetContentBase();
        assert(pContentBase);

        nLength = sprintf_s(szBuf, sizeof(szBuf), "%s%s", pContentBase, pValue);
        if (nLength < 0)
                return Z3_EINVAL;

        pRequest = new RtspMsg;
        if (pRequest == NULL)
                return Z3_ENOMEM;
        Z3_OBJ_ADDREF(pRequest);

        result= pRequest->InitRequest(RTSP_SETUP, szBuf, nLength);
        assert(result == Z3_EOK);
        
        pMediaSession = m_pRtspMedia->CreateMediaSession(pSDPMedia, m_hIOCP);
        if (!pMediaSession)
        {
                TRACE_ERROR("Failed to create RTP session from SDPMedia(0x%p)\r\n", pSDPMedia);
                return Z3_RTSP_EMEDIASESSION;
        }
        
        pMediaSession->GetPort(&nRtpPort, &nRtcpPort);
        assert(nRtpPort != 0 && nRtcpPort != 0);
        assert(nRtcpPort == nRtpPort + 1);
        assert(nRtpPort % 2 == 0);

        switch (pMediaSession->GetTransferMode())
        {
        case RTP_OVER_RTSP:
                nLength = sprintf_s(szBuf, sizeof(szBuf), "RTP/AVP/TCP;unicast;interleaved=%d-%d", 
                        nRtpPort, nRtcpPort);
                break;
        case RTP_OVER_UDP:
                nLength = sprintf_s(szBuf, sizeof(szBuf), "RTP/AVP;unicast;client_port=%d-%d", 
                        nRtpPort, nRtcpPort);
                break;
        case RTP_OVER_TCP:
                assert(false);
                break;
        default:
                break;
        }

        result = pRequest->AddHeader(RTSP_HDR_TRANSPORT, szBuf, nLength + 1);
        assert(result == Z3_EOK);

        if (m_pszSessionId)
                pRequest->AddHeader(RTSP_HDR_SESSION, m_pszSessionId, strlen(m_pszSessionId) + 1);

        nLength = sprintf_s(szBuf, sizeof(szBuf), "%d", ++m_nCSeqPrevious);
        assert(nLength > 0);

        result = pRequest->AddHeader(RTSP_HDR_CSEQ, szBuf, nLength + 1);
        assert(result == Z3_EOK);

        nError = WriteMsg(pRequest);
        if (nError == 0)
                m_methodPrevious = RTSP_SETUP;

        Z3_OBJ_RELEASE(pRequest);

        return nError;

}

int RtspSession::SendPLAY()
{
        char    szBuf[256], *pContentBase;
        int     nLength, nError;
        RtspMsg *pRequest;
        int     result;

        assert(m_pRtspMedia);
        pContentBase = m_pRtspMedia->GetContentBase();
        assert(pContentBase);

        nLength = sprintf_s(szBuf, sizeof(szBuf), "%s", pContentBase);
        if (nLength < 0)
                return Z3_EINVAL;

        pRequest = new RtspMsg;
        if (pRequest == NULL)
                return Z3_ENOMEM;

        result= pRequest->InitRequest(RTSP_PLAY, szBuf, nLength);
        assert(result == Z3_EOK);

        nLength = sprintf_s(szBuf, sizeof(szBuf), "%d", ++m_nCSeqPrevious);
        assert(nLength > 0);

        result = pRequest->AddHeader(RTSP_HDR_CSEQ, szBuf, nLength + 1);
        assert(result == Z3_EOK);

        nLength = sprintf_s(szBuf, sizeof(szBuf), "npt=0.000-");
        assert(nLength > 0);

        result = pRequest->AddHeader(RTSP_HDR_RANGE, szBuf, nLength + 1);
        assert(result == Z3_EOK);

        result = pRequest->AddHeader(RTSP_HDR_SESSION, m_pszSessionId, strlen(m_pszSessionId) + 1);
        assert(result == Z3_EOK);

        nError = WriteMsg(pRequest);
        if (nError == 0)
                m_methodPrevious = RTSP_PLAY;

        Z3_OBJ_RELEASE(pRequest);

        return nError;
}