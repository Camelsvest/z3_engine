#include "z3_common.hpp"
#include "z3_rtp_udp_session.hpp"
#include "z3_rtsp_def.hpp"
#include "z3_rtsp_error.hpp"
#include "z3_rtsp_ids.hpp"
#include "z3_rtcp_msg.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

RTPUDPParser::RTPUDPParser()
        : m_pRTPPacket(NULL)
        , m_pH264Frame(NULL)
        , m_bFrameCompleted(false)
{
        m_pH264Frame = new H264Frame;
        m_pH264Frame->Allocate(H264_MAX_FRAME_SIZE);
}

RTPUDPParser::~RTPUDPParser()
{
        Z3_DELETE_OBJ(m_pRTPPacket);
        Z3_OBJ_RELEASE(m_pH264Frame);
}

int RTPUDPParser::Parse(const char *pBuf, uint32_t nLen, void *pData, Msg **pMsg /* output */)
{        
        unsigned char   *pHeaderStart;
        unsigned char   nalUnitType;
        uint32_t        nPayloadSize;
        uint32_t        nExpectedHeaderSize;
        uint32_t        nDataSize;
        unsigned char   startBit, endBit;
        bool            bNalBegin, bNalEnd;
        unsigned long   lTimestamp;

        TRACE_DEBUG("Receive a RTP packet, SIZE = %u\r\n", nLen);
        TRACE_DEBUG("RTP->Ver = %u\r\n", RTP_PKT_VER(pBuf));
        TRACE_DEBUG("RTP->Ext = %u\r\n", RTP_PKT_EXT(pBuf));
        TRACE_DEBUG("RTP->SEQ = %u\r\n", RTP_PKT_SEQ(pBuf));
        TRACE_DEBUG("RTP->TIMESTAMP = 0x%X\r\n", RTP_PKT_TS(pBuf));

        if (m_bFrameCompleted)
        {
                m_pH264Frame->Reset();
                m_bFrameCompleted = false;
        }

        if (m_pRTPPacket == NULL)
        {
                m_pRTPPacket = new RTPPacket;
                assert(m_pRTPPacket);
        }

        m_pRTPPacket->InitPacket((char *)pBuf, nLen);

        pHeaderStart = (unsigned char *)m_pRTPPacket->GetPayload();
        nPayloadSize = m_pRTPPacket->GetPayloadSize();

        nExpectedHeaderSize = 0;
        
        nalUnitType = (pHeaderStart[0] & 0x1F);
        switch (nalUnitType)
        {
        case 24:        // STAP-A
                {
                        nExpectedHeaderSize = 1;        // discard the type byte
                        break;
                }
        case 25: case 26: case 27:      // STAP-B, MTAP16, or MTAP24
                {
                        nExpectedHeaderSize = 3;
                        break;
                }
        case 28: case 29:       // FU-A or FU-B
                {
                        startBit = (pHeaderStart[1] & 0x80);
                        endBit = (pHeaderStart[1] & 0x40);

                        if (startBit)
                        {
                                nExpectedHeaderSize = 1;
                                assert(nPayloadSize > nExpectedHeaderSize);

                                pHeaderStart[1] = (pHeaderStart[0] & 0xE0) + (pHeaderStart[1] & 0x1F);
                                bNalBegin = true;
                        }
                        else
                        {
                                nExpectedHeaderSize = 2;
                                assert(nPayloadSize > nExpectedHeaderSize);

                                bNalBegin = false;
                        }

                        bNalEnd = (endBit != 0);
                        break;
                }
        default:        // This packet contains one or more complete, decodable NAL units
                bNalBegin = true;
                bNalEnd = true;
                break;
        }

        pHeaderStart += nExpectedHeaderSize;

        if (bNalBegin)
                m_pH264Frame->AddNalStartCode();

        nDataSize = m_pH264Frame->FillData((char *)pHeaderStart, nPayloadSize - nExpectedHeaderSize);
        assert(nDataSize >= 0);

        lTimestamp = m_pH264Frame->GetTimestamp();
        if (0 == lTimestamp)
                m_pH264Frame->SetTimestamp(m_pRTPPacket->GetTimestamp());       // new H264 frame
        else
        {
                if (!bNalEnd)
                {
                        if (lTimestamp != m_pRTPPacket->GetTimestamp())
                        {
                                TRACE_ERROR("Timestamp mismatched, maybe packet loose !\r\n");
                        }
                }
        }

        if (m_pRTPPacket->IsMarker())
        {
                m_bFrameCompleted = true;
                *pMsg = m_pH264Frame;

                return Z3_EOK;       // Frame completed, which maybe include multiple NAL units
        }

        return Z3_EINTR;      // Frame is incompleted
}

RTPUDPSession::RTPUDPSession(HANDLE hIOCP)
        : UDPSession(hIOCP, RTP_UDP_SESSION_ID)
        , m_pParser(NULL)
{
}

RTPUDPSession::~RTPUDPSession()
{
        Z3_OBJ_RELEASE(m_pParser);
}

ProtoParser* RTPUDPSession::GetProtoParser()
{
        if (m_pParser == NULL)
                m_pParser = new RTPUDPParser;

        return m_pParser;
}

int RTPUDPSession::Dispatch(Msg *pMsg, void *pData)
{
        H264Frame       *pH264Frame;

        pH264Frame = dynamic_cast<H264Frame *>(pMsg);

        TRACE_DEBUG("Received a H264 frame now, size %ul, timestamp %ul\r\n",
                pH264Frame->GetFrameSize(), pH264Frame->GetTimestamp());

        return 0;
}

RTCPUDPParser::RTCPUDPParser()
{
}

RTCPUDPParser::~RTCPUDPParser()
{
}

int RTCPUDPParser::Parse(const char *pBuf, uint32_t nLen, void *pData, Msg **pMsg)
{
        RTCPPacket      *pPacket;
        uint16_t        nLength;

        nLength = RTCP_PKT_LENGTH(pBuf);

        while (nLength < nLen)
        {
                pPacket = new RTCPPacket;
                pPacket->InitPacket((char *)pBuf, nLength);

                *pMsg = pPacket;
        }

        return 0;
}

RTCPUDPSession::RTCPUDPSession(HANDLE hIOCP)
        : UDPSession(hIOCP, RTCP_UDP_SESSION_ID)
        , m_pParser(NULL)
{
}

RTCPUDPSession::~RTCPUDPSession()
{
        Z3_OBJ_RELEASE(m_pParser);
}

ProtoParser* RTCPUDPSession::GetProtoParser()
{
        if (m_pParser == NULL)
                m_pParser = new RTCPUDPParser;

        return m_pParser;
}

int RTCPUDPSession::Dispatch(Msg *pMsg, void *pData)
{
        return 0;
}
