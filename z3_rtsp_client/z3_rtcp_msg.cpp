#include "z3_common.hpp"
#include "z3_rtcp_msg.hpp"
#include "z3_rtsp_ids.hpp"
#include "z3_rtsp_def.hpp"

using namespace Z3;

RTCPPacket::RTCPPacket()
        : m_pPacket(NULL)
        , m_nPacketSize(0)
{
}

RTCPPacket::~RTCPPacket()
{
        Z3_FREE_POINTER(m_pPacket);
}

void RTCPPacket::InitPacket(char *pBuf, size_t nBufSize)
{
        Z3_FREE_POINTER(m_pPacket);
        m_pPacket = (char *)z3_malloc(nBufSize);
        if (m_pPacket)
        {
                m_nPacketSize = nBufSize;
                memcpy(m_pPacket, pBuf, nBufSize);
        }
}

unsigned char RTCPPacket::GetPayloadType()
{
        assert(m_pPacket);

        return RTCP_PKT_PT(m_pPacket);
}

uint16_t RTCPPacket::GetPacketLength()
{
        assert(m_pPacket);
        assert(RTCP_PKT_LENGTH(m_pPacket) <= m_nPacketSize);

        return RTCP_PKT_LENGTH(m_pPacket);
}

RTCPPacketList::RTCPPacketList()
        : Msg(RTCP_MSG_ID)
{
}

RTCPPacketList::~RTCPPacketList()
{
        RTCPPacket *pPacket;
        std::list<RTCPPacket *>::iterator itera;

        for (itera = m_lstRtcpPackets.begin(); itera != m_lstRtcpPackets.end(); itera++)
        {
                pPacket = *itera;
                delete pPacket;
        }
}

unsigned int RTCPPacketList::ProtoID()
{
        return RTCP_PROTO_ID;
}

int RTCPPacketList::Push(RTCPPacket *pPacket)
{
        m_lstRtcpPackets.push_back(pPacket);

        return m_lstRtcpPackets.size();
}

RTCPPacket* RTCPPacketList::Pop(void)
{
        RTCPPacket *pPacket = NULL;

        if (!m_lstRtcpPackets.empty())
        {
                pPacket = *m_lstRtcpPackets.begin();
                m_lstRtcpPackets.pop_front();
        }

        return pPacket;
}


RB::RB()
        : m_pBlock(NULL)
        , m_nBlockSize(0)
{
}

RB::~RB()
{
}

void RB::InitBlock(char *pBlock, size_t nBlockSize)
{
        m_pBlock = pBlock;

        assert(nBlockSize >= sizeof(RTCP_RR_BLOCK));
        m_nBlockSize = nBlockSize;
}

uint32_t RB::GetSSRC()
{
        assert(m_pBlock);

        return RB_BLK_SSRC(m_pBlock);
}

uint32_t RB::GetLostRate()
{
        assert(m_pBlock);

        return RB_BLK_LOST_RATE(m_pBlock);
}

uint32_t RB::GetLost()
{
        assert(m_pBlock);

        return RB_BLK_LOST(m_pBlock);
}

uint32_t RB::GetJitter()
{
        assert(m_pBlock);

        return RB_BLK_JITTER(m_pBlock);
}

uint32_t RB::GetLSR()
{
        assert(m_pBlock);

        return RB_BLK_LSR(m_pBlock);
}

uint32_t RB::GetDLSR()
{
        assert(m_pBlock);

        return RB_BLK_DLSR(m_pBlock);
}

SR::SR()
{
}

SR::~SR()
{
}

void SR::InitPacket(char *pBuf, size_t nBufSize)
{
        RB              reportBlock;
        uint32_t        nOffset, nCount;
        uint16_t        nPktLength;

        RTCPPacket::InitPacket(pBuf, nBufSize);

        nCount = GetRC();
        if (nCount > 0)
        {
                nPktLength = GetPacketLength();
                assert(nPktLength >= (nCount * 24 + 28));

                while (nCount > 0)
                {
                        --nCount;
                        nOffset = 28 + nCount * 24;

                        reportBlock.InitBlock(pBuf + nOffset, 24);
                        m_vectorRBs.push_back(reportBlock);
                }
        }
        
        return;
}

uint32_t SR::GetRC()
{
        assert(m_pPacket);

        return RTCP_PKT_RC(m_pPacket);
}

uint32_t SR::GetSSRC()
{
        uint32_t        ssrc;
        RTCP_SR_HEADER  *pHDR;

        assert(m_nPacketSize >= sizeof(RTCP_SR_HEADER));
        pHDR = (RTCP_SR_HEADER *)m_pPacket;

        ssrc = ntohl(pHDR->ssrc);

        return ssrc;
}

uint64_t SR::GetNTPTimestamp()
{
        uint64_t        ntp_ts;
        ULARGE_INTEGER  ui;
        RTCP_SR_HEADER  *pHDR;

        assert(m_nPacketSize >= sizeof(RTCP_SR_HEADER));
        pHDR = (RTCP_SR_HEADER *)m_pPacket;

        ui.HighPart = ntohl(pHDR->ntp_hi);
        ui.LowPart = ntohl(pHDR->ntp_lo);
        ntp_ts = ui.QuadPart;

        return ntp_ts;
}

unsigned long SR::GetRTPTimestamp()
{
        RTCP_SR_HEADER  *pHDR;
        unsigned long   ts;

        assert(m_nPacketSize >= sizeof(RTCP_SR_HEADER));
        pHDR = (RTCP_SR_HEADER *)m_pPacket;

        ts = ntohl(pHDR->rtp_ts);

        return ts;
}

uint32_t SR::GetSenderPktCount()
{
        RTCP_SR_HEADER  *pHDR;
        uint32_t        nCount;

        assert(m_nPacketSize >= sizeof(RTCP_SR_HEADER));
        pHDR = (RTCP_SR_HEADER *)m_pPacket;

        nCount = ntohl(pHDR->pkt_sent);

        return nCount;
}

uint32_t SR::GetSenderBytes()
{
        RTCP_SR_HEADER  *pHDR;
        uint32_t        nBytes;

        assert(m_nPacketSize >= sizeof(RTCP_SR_HEADER));
        pHDR = (RTCP_SR_HEADER *)m_pPacket;

        nBytes = ntohl(pHDR->oct_sent);

        return nBytes;
}

uint32_t SR::GetRBCount()
{
        return m_vectorRBs.size();
}

RB* SR::GetRB(uint32_t nIndex)
{
        if (nIndex < m_vectorRBs.size())
        {
                RB &report = m_vectorRBs.at(nIndex);

                return &report;
        }
        else
                return NULL;
}

RR::RR()
{
}

RR::~RR()
{
}

void RR::InitPacket(char *pBuf, size_t nBufSize)
{
        RB              reportBlock;
        uint32_t        nOffset, nCount;
        uint16_t        nPktLength;

        RTCPPacket::InitPacket(pBuf, nBufSize);

        nCount = GetRC();
        if (nCount > 0)
        {
                nPktLength = GetPacketLength();
                assert(nPktLength >= (nCount * 24 + 28));

                while (nCount > 0)
                {
                        --nCount;
                        nOffset = 28 + nCount * 24;

                        reportBlock.InitBlock(pBuf + nOffset, 24);
                        m_vectorRBs.push_back(reportBlock);
                }
        }
        
        return;
}

uint32_t RR::GetRC()
{
        assert(m_pPacket);

        return RTCP_PKT_RC(m_pPacket);
}

uint32_t RR::GetRBCount()
{
        return m_vectorRBs.size();
}

RB* RR::GetRB(uint32_t nIndex)
{
        if (nIndex < m_vectorRBs.size())
        {
                RB &report = m_vectorRBs.at(nIndex);

                return &report;
        }
        else
                return NULL;
}

SrcDesc::SrcDesc()
        : m_pChunk(NULL)
        , m_nChunkSize(0)
{
}

SrcDesc::~SrcDesc()
{
}

void SrcDesc::InitChunk(char *pChunk, size_t nChunkSize)
{
        m_pChunk = pChunk;
        m_nChunkSize = nChunkSize;

        assert(m_nChunkSize >= 6);
}

uint32_t SrcDesc::GetSSRC()
{
        uint32_t *pSsrc, SSRC;

        assert(m_pChunk);
        pSsrc = (uint32_t *)m_pChunk;

        SSRC = ntohl(*pSsrc);

        return SSRC;
}

uint8_t SrcDesc::GetType()
{
        RTCP_SRC_DESC *pDesc;

        assert(m_pChunk);
        pDesc = (RTCP_SRC_DESC *)m_pChunk;
        
        return pDesc->type;
}

uint8_t SrcDesc::GetLength()
{
        RTCP_SRC_DESC *pDesc;

        assert(m_pChunk);
        pDesc = (RTCP_SRC_DESC *)m_pChunk;
        
        return pDesc->len;
}

uint8_t SrcDesc::GetItemName(char *pBuf, size_t nBufSize)
{
        uint32_t         nLen;
        RTCP_SRC_DESC   *pDesc;

        assert(m_pChunk);
        pDesc = (RTCP_SRC_DESC *)m_pChunk;

        nLen = GetLength();
        assert(nLen < nBufSize);

        if (!strncpy_s(pBuf, nBufSize, (char *)(pDesc->data), nLen))
                return nLen;
        else
                return 0;
}

SDES::SDES()
{
}

SDES::~SDES()
{
}

void SDES::InitPacket(char *pBuf, size_t nBufSize)
{
        uint32_t nCount, nOffset, nPktLength;
        SrcDesc desc;

        RTCPPacket::InitPacket(pBuf, nBufSize);

        nCount = GetSC();

        if (nCount > 0)
        {
                nPktLength = GetPacketLength();
                assert(nPktLength > 4);

                nOffset = 0;
                while (nCount > 0)
                {
                        --nCount;
                        nOffset += 4;

                        desc.InitChunk(pBuf + nOffset, nBufSize - nOffset);
                        m_vectorDESCs.push_back(desc);

                        nOffset += 2;
                        nOffset += desc.GetLength();

                        if (nOffset % 4)        // 必须保证4字节对齐
                                nOffset = ((nOffset + 4) / 4) * 4;
                }
        }
}

uint32_t SDES::GetSC()
{
        assert(m_pPacket);

        return RTCP_PKT_RC(m_pPacket);
}