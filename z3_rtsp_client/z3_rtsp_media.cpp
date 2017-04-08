#include "z3_common.hpp"
#include "z3_rtsp_media.hpp"
#include "z3_rtp_udp_session.hpp"
#include "z3_rtcp_udp_session.hpp"
#include "z3_media_ports.hpp"
#include "z3_rtsp_def.hpp"

using namespace Z3;

//#ifdef Z3_MEM_DEBUG
//#define new z3_debug_new
//#endif

MediaSession::MediaSession(HANDLE hIOCP)
        : m_hIOCP(hIOCP)
        , m_nRTPPort(0)
        , m_nRTCPPort(0)
        , m_pRTPSession(NULL)
        , m_pRTCPSession(NULL)
        , m_nMediaType(UNKNOWN_TYPE)
        , m_nTransferMode(UNKNOWN_MODE)
{
}

MediaSession::~MediaSession()
{
        Z3_OBJ_RELEASE(m_pRTPSession);
        Z3_OBJ_RELEASE(m_pRTCPSession);
}

int MediaSession::Init(SDPMedia *pSDPMedia)
{
        int             nError;
        MediaPorts      *pMediaPorts;
        const char      *pszMedia;

        assert(pSDPMedia);
        pszMedia = pSDPMedia->GetMedia();
        if (!strcmp(pszMedia, "video"))
                m_nMediaType = VIDEO_TYPE;
        else if (!strcmp(pszMedia, "audio"))
                m_nMediaType = AUDIO_TYPE;
        else
                m_nMediaType = OTHER_TYPE;

        m_nTransferMode = RTP_OVER_UDP;   // default mode

        pMediaPorts = MediaPorts::GetInstance();
        if (pMediaPorts == NULL)
                return ENOMEM;

        nError = pMediaPorts->GetPairPorts(&m_nRTPPort, &m_nRTCPPort);
        if (nError)
        {
                TRACE_ERROR("No port can be allocated now!\r\n");
                return nError;
        }

        assert(m_pRTPSession == NULL && m_pRTCPSession == NULL);
        m_pRTPSession = z3_debug_new RTPUDPSession(m_hIOCP);
        if (m_pRTPSession == NULL)
        {
                TRACE_ERROR("No memory allocated for RTPUDPSession object now\r\n");
                return ENOMEM;
        }
        Z3_OBJ_ADDREF(m_pRTPSession);

        nError = m_pRTPSession->Init(m_nRTPPort);
        if (nError != 0)
        {
                TRACE_ERROR("Failed to init RTPUDPSession on port %u, error code is %d\r\n", m_nRTPPort, nError);
                return nError;
        }
        
        nError = m_pRTPSession->StartRead(UDP_RECV_TIMEOUT);
        if (nError != 0)
        {
                TRACE_ERROR("Failed to Start Receiving operation on RTPUDPSession object 0x%p\r\n", m_pRTPSession);
                return nError;
        }

        m_pRTCPSession = z3_debug_new RTCPUDPSession(m_hIOCP);
        if (m_pRTCPSession == NULL)
        {
                TRACE_ERROR("No memory allocated for RTCPUDPSession object now\r\n");
                return ENOMEM;
        }
        Z3_OBJ_ADDREF(m_pRTCPSession);

        nError = m_pRTCPSession->Init(m_nRTCPPort);
        if (nError != 0)
        {
                TRACE_ERROR("Failed to init RTCPUDPSession on port %u, error code is %d\r\n", m_nRTCPPort, nError);
                return nError;
        }

        nError = m_pRTCPSession->StartRead(UDP_RECV_TIMEOUT);
        if (nError != 0)
        {
                TRACE_ERROR("Failed to Start Receiving operation on RTCPUDPSession object 0x%p\r\n", m_pRTPSession);
                return nError;
        }

        return nError;
}

void MediaSession::GetPort(uint16_t *pRTPPort, uint16_t *pRTCPPort)
{
        *pRTPPort = m_nRTPPort;
        *pRTCPPort = m_nRTCPPort;
}

RtspMedia::RtspMedia()
        : m_pszContentBase(NULL)
        , m_pMediaSessions(NULL)
        , m_pSDP(NULL)
        , m_nTrackCount(0xFFFFFFFF)
        , m_nRequestTrack(0xFFFFFFFF)
{
}

RtspMedia::~RtspMedia()
{
        std::list<MediaSession *>::iterator        itera;
        MediaSession                               *pSession;

        for (itera = m_pMediaSessions->begin(); itera != m_pMediaSessions->end(); itera++)
        {
                pSession = *itera;
                Z3_DELETE_OBJ(pSession);
        }
        Z3_DELETE_OBJ(m_pMediaSessions);

        Z3_FREE_POINTER(m_pszContentBase);
        Z3_DELETE_OBJ(m_pSDP);
}

bool RtspMedia::Init(
        char                    *pszContentBase,
        unsigned int            nTracks,
        unsigned int            nRequestTrack,
        SDPMessage              *pSDP)
{
        m_nTrackCount = nTracks;
        m_nRequestTrack = nRequestTrack;

        m_pSDP = pSDP;

        m_pszContentBase = z3_strdup(pszContentBase);
        if (!m_pszContentBase)
        {
                TRACE_ERROR("Failed to duplicate ContentBase %s\r\n", pszContentBase);
                return false;
        }

        return true;
}

bool RtspMedia::AddMediaSession(MediaSession *pSession)
{
        if (m_pMediaSessions == NULL)
                // z2_new cannot used for STL object, if want you must overload allocator of STL
                m_pMediaSessions = new std::list<MediaSession *>;    

        if (m_pMediaSessions)
        {
                m_pMediaSessions->push_back(pSession);
                return true;
        }

        return false;
}

MediaSession* RtspMedia::CreateMediaSession(SDPMedia *pMedia, HANDLE hIOCP)
{
        MediaSession *pMediaSession;

        assert(pMedia);
        pMediaSession = z3_debug_new MediaSession(hIOCP);
        if (pMediaSession == NULL)
                return NULL;

        if (Z3_EOK == pMediaSession->Init(pMedia))
                AddMediaSession(pMediaSession);
        else
                Z3_DELETE_OBJ(pMediaSession);

        // 上层应用如果不使用pRtpSession，应调用Z2_OBJ_RELEASE(pRtpSession)释放
        return pMediaSession;
}