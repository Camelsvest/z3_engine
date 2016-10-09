#include "z3_common.hpp"
#include "z3_rtp_msg.hpp"
#include "z3_rtsp_def.hpp"
#include "z3_rtsp_ids.hpp"

using namespace Z3;

RTPPacket::RTPPacket()
        : m_pPacket(NULL)
        , m_nPacketSize(0)
{
}

RTPPacket::~RTPPacket()
{
}

void RTPPacket::InitPacket(char *pBuf, size_t nBufSize)
{
        m_pPacket = pBuf;
        m_nPacketSize = nBufSize;
}

unsigned char* RTPPacket::GetPayload()
{
        RTP_HDR         *pHdr;
        RTP_EXT_HDR     *pExtHdr;
        unsigned char   *pStart;

        pHdr = (RTP_HDR *)m_pPacket;
        if (pHdr->extension)
        {
                pStart = (unsigned char *)RTP_PKT_DATA(m_pPacket);

                pExtHdr = (RTP_EXT_HDR *)pStart;
                pStart  += sizeof(RTP_EXT_HDR) + 4 * ntohs(pExtHdr->length);

                return pStart;
        }
        else
        {
                return (unsigned char *)RTP_PKT_DATA(m_pPacket);
        }
}

uint32_t RTPPacket::GetPayloadSize()
{
        char *pPayload;

        pPayload = (char *)GetPayload();

        return m_pPacket + m_nPacketSize - pPayload;
}

unsigned long RTPPacket::GetTimestamp()
{
        RTP_HDR         *pHdr;

        pHdr = (RTP_HDR *)m_pPacket;

        return ntohl(pHdr->timestamp);
}

bool RTPPacket::IsMarker()
{
        RTP_HDR         *pHdr;

        pHdr = (RTP_HDR *)m_pPacket;
        
        return (pHdr->marker == 1);
}


RtpMsg::RtpMsg()
        : Msg(RTP_MSG_ID)
        , m_pPacket(NULL)
        , m_nPacketSize(0)
{
}

RtpMsg::~RtpMsg()
{
        Z3_FREE_POINTER(m_pPacket);
}

uint32_t RtpMsg::ProtoID()
{
        return RTP_PROTO_ID;
}

bool RtpMsg::ToStringImpl(char **ppbuf, uint32_t *pnSize)
{
        TRACE_ERROR("This function is useless in object RtpMsg\r\n");
        return false;
}

int RtpMsg::FillBuffer(const char *pRtpPacket, uint32_t nPktSize)
{
        assert(m_pPacket == NULL && m_nPacketSize == 0);

        m_pPacket = (char *)z3_malloc(nPktSize);
        if (m_pPacket)
        {
                memcpy(m_pPacket, pRtpPacket, nPktSize);
                return 0;
        }
        
        return ENOMEM;
}

unsigned char* RtpMsg::GetPayload()
{
        RTP_HDR         *pHdr;
        RTP_EXT_HDR     *pExtHdr;
        unsigned char   *pStart;

        pHdr = (RTP_HDR *)m_pPacket;
        if (pHdr->extension)
        {
                pStart = (unsigned char *)RTP_PKT_DATA(m_pPacket);

                pExtHdr = (RTP_EXT_HDR *)pStart;
                pStart  += sizeof(RTP_EXT_HDR) + 4 * ntohs(pExtHdr->length);

                return pStart;
        }
        else
        {
                return (unsigned char *)RTP_PKT_DATA(m_pPacket);
        }
}

uint32_t RtpMsg::GetPayloadSize()
{
        char *pPayload;

        pPayload = (char *)GetPayload();

        return m_pPacket + m_nPacketSize - pPayload;
}