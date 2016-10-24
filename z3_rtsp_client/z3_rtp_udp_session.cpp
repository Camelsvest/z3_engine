#include "z3_common.hpp"
#include "z3_rtp_udp_session.hpp"
#include "z3_rtsp_def.hpp"
#include "z3_rtsp_error.hpp"
#include "z3_rtsp_ids.hpp"
#include "z3_rtcp_msg.hpp"
#include "rtcp_from_spec.h"

#define IP_UDP_HDR_SIZE 28

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
        RTCPPacketList  *pList;
        RTCPPacket      *pPacket;
        uint16_t        nLength, nOffset;
        int             nResult = Z3_EOK;

        nLength = RTCP_PKT_LENGTH(pBuf);

        pList = new RTCPPacketList;
        assert(pList);

        nOffset = 0;
        while (nLength <= (nLen - nOffset))
        {
                pPacket = new RTCPPacket;
                pPacket->InitPacket((char *)pBuf + nOffset, nLength);

                pList->Push(pPacket);

                nOffset += nLength;
                nLength = RTCP_PKT_LENGTH((pBuf + nOffset));
        }

        *pMsg = pList;

        if (pList->IsEmpty())
        {
                nResult = Z3_ERROR;
                Z3_OBJ_RELEASE(pList);
        }

        return nResult;
}

RTCPMemberDB::RTCPMemberDB(RTCPUDPSession& instance) 
        : m_RTCPSession(instance)
        , m_nNumMembers(1 /*ourself*/)
{
}

RTCPMemberDB::~RTCPMemberDB()
{
}

bool RTCPMemberDB::IsMember(uint32_t ssrc) const
{
        std::map<uint32_t, uint32_t>::const_iterator itera;

        itera = m_map.find(ssrc);
        
        return (itera != m_map.end());
}

bool RTCPMemberDB::NoteMembership(uint32_t ssrc, uint32_t curTimeCount)
{
        typedef std::pair<uint32_t, uint32_t> UINT32_PAIR;
        std::map<uint32_t, uint32_t>::iterator itera;

        bool isNew = !IsMember(ssrc);

        if (isNew)
        {
                ++m_nNumMembers;
                m_map.insert(UINT32_PAIR(ssrc, curTimeCount));
        }
        else
        {
                itera = m_map.find(ssrc);
                itera->second = curTimeCount;
        }
       
        return isNew;
}

bool RTCPMemberDB::Remove(uint32_t ssrc)
{
        int nCount;

        nCount = m_map.erase(ssrc);
        if (nCount > 0)
                --m_nNumMembers;

        return (nCount > 0);
}

void RTCPMemberDB::ReapOldMembers(uint32_t threshold)
{
        std::map<uint32_t, uint32_t>::iterator itera;
        bool foundOldMember;
        uint32_t timeCount, oldSSRC = 0;

        do {
                foundOldMember = false;

                for (itera = m_map.begin(); itera != m_map.end(); itera++)
                {
                        timeCount = itera->second;
                        if (timeCount < threshold)
                        {
                                oldSSRC = itera->first;
                                foundOldMember = true;
                                break;
                        }
                }

                if (foundOldMember) 
                        m_RTCPSession.RemoveSSRC(oldSSRC, true);

        } while (foundOldMember);
}

RTCPUDPSession::RTCPUDPSession(HANDLE hIOCP)
        : UDPSession(hIOCP, RTCP_UDP_SESSION_ID)
        , m_pParser(NULL)
        , m_pStatsDB(NULL)
        , m_pKnownMembers(NULL)
        , m_nLastReceivedSSRC(0)
        , m_nLastReceivedSize(0)
        , m_nPayloadType(0)
        , m_nOutgoingReportCount(0)
        , m_dbAveRTCPSize(0.0)
        , m_dbPrevReportTime(0.0)
        , m_dbNextReportTime(0.0)
{
}

RTCPUDPSession::~RTCPUDPSession()
{
        Z3_OBJ_RELEASE(m_pParser);
        Z3_DELETE_OBJ(m_pStatsDB);
        Z3_DELETE_OBJ(m_pKnownMembers);
}

ProtoParser* RTCPUDPSession::GetProtoParser()
{
        if (m_pParser == NULL)
        {
                m_pParser = new RTCPUDPParser;

                assert(m_pStatsDB == NULL);
                m_pStatsDB = new RTPReceptionStatsDB;

                assert(m_pKnownMembers == NULL);
                m_pKnownMembers = new RTCPMemberDB(*this);
        }

        return m_pParser;
}

int RTCPUDPSession::Dispatch(Msg *pMsg, void *pData)
{
        RTCPPacketList  *pList;
        RTCPPacket      *pPacket;
        SR              *pSR;
        uint32_t        nTotalBytes, nSSRC = 0;

        pList = dynamic_cast<RTCPPacketList *>(pMsg);
        assert(pList);

        assert(m_pStatsDB);
        pPacket = pList->Pop();
        while (pPacket)
        {
                switch (pPacket->GetPayloadType())
                {
                case RTCP_SR:
                        pSR = dynamic_cast<SR *>(pPacket);
                        assert(pSR);

                        nSSRC = pSR->GetSSRC();
                        m_pStatsDB->ProcessIncomingSR(pSR);
                        break;
                case RTCP_RR:
                        TRACE_WARN("Unhandled RTCP RR!\r\n");
                        break;
                case RTCP_BYE:
                        TRACE_WARN("Unhandled RTCP BYE!\r\n");
                        break;
                case RTCP_APP:
                        TRACE_WARN("Unhandled RTCP APP!\r\n");
                        break;
                case RTCP_SDES:
                        TRACE_WARN("Unhandled RTCP SDES!\r\n");
                        break;
                default:
                        TRACE_WARN("Unhandled RTCP PT %u!\r\n", pPacket->GetPayloadType());
                        break;
                }
                pPacket = pList->Pop();

                nTotalBytes = IP_UDP_HDR_SIZE + pPacket->GetPacketLength();

                OnReceive(pPacket->GetPayloadType(), nTotalBytes, nSSRC);
        }

        Z3_OBJ_RELEASE(pList);
        return 0;
}

extern "C" int gettimeofday(struct timeval* tp, int *tz);

static double dTimeNow()
{
    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);

    return (double) (timeNow.tv_sec + timeNow.tv_usec/1000000.0);
}

void RTCPUDPSession::OnReceive(int nPayloadType, uint32_t nTotalBytes, uint32_t nSSRC)
{
        int     nMembers;
        int     nSenders;

        m_nPayloadType = nPayloadType;
        m_nLastReceivedSize = nTotalBytes;
        m_nLastReceivedSSRC = nSSRC;

        nMembers = NumMembers();
        nSenders = 0;   // we're a client;

        ::OnReceive(this, this, &nMembers, &nSenders, &m_nPrevNumMembers,
                &m_dbAveRTCPSize, &m_dbPrevReportTime, dTimeNow(), m_dbNextReportTime);

}

int RTCPUDPSession::CheckNewSSRC()
{
        return m_pKnownMembers->NoteMembership(m_nLastReceivedSSRC, m_nOutgoingReportCount);        
}

void RTCPUDPSession::RemoveSSRC(uint32_t nSSRC, bool bRemoveStats)
{
        m_pKnownMembers->Remove(nSSRC);

        if (bRemoveStats)
        {
                assert(false);
                // to be done ... 
        }
}

uint32_t RTCPUDPSession::NumMembers()
{
        if (m_pKnownMembers == NULL)
                return 0;

        return m_pKnownMembers->NumMembers();
}

////////// Implementation of routines imported by the "rtcp_from_spec" C code

extern "C" void Schedule(double nextTime, event e) {
  RTCPUDPSession* instance = (RTCPUDPSession*)e;
  if (instance == NULL) return;

  instance->schedule(nextTime);
}

extern "C" void Reschedule(double nextTime, event e) {
  RTCPUDPSession* instance = (RTCPUDPSession*)e;
  if (instance == NULL) return;

  instance->reschedule(nextTime);
}

extern "C" void SendRTCPReport(event e) {
  RTCPUDPSession* instance = (RTCPUDPSession*)e;
  if (instance == NULL) return;

  instance->sendReport();
}

extern "C" void SendBYEPacket(event e) {
  RTCPUDPSession* instance = (RTCPUDPSession*)e;
  if (instance == NULL) return;

  instance->sendBYE();
}

extern "C" int TypeOfEvent(event e) {
  RTCPUDPSession* instance = (RTCPUDPSession*)e;
  if (instance == NULL) return EVENT_UNKNOWN;

  return instance->typeOfEvent();
}

extern "C" int SentPacketSize(event e) {
  RTCPUDPSession* instance = (RTCPUDPSession*)e;
  if (instance == NULL) return 0;

  return instance->sentPacketSize();
}

extern "C" int PacketType(packet p) {
  RTCPUDPSession* instance = (RTCPUDPSession*)p;
  if (instance == NULL) return PACKET_UNKNOWN_TYPE;

  return instance->packetType();
}

extern "C" int ReceivedPacketSize(packet p) {
  RTCPUDPSession* instance = (RTCPUDPSession*)p;
  if (instance == NULL) return 0;

  return instance->receivedPacketSize();
}

extern "C" int NewMember(packet p) {
  RTCPUDPSession* instance = (RTCPUDPSession*)p;
  if (instance == NULL) return 0;

  return instance->checkNewSSRC();
}

extern "C" int NewSender(packet /*p*/) {
  return 0; // we don't yet recognize senders other than ourselves #####
}

extern "C" void AddMember(packet /*p*/) {
  // Do nothing; all of the real work was done when NewMember() was called
}

extern "C" void AddSender(packet /*p*/) {
  // we don't yet recognize senders other than ourselves #####
}

extern "C" void RemoveMember(packet p) {
  RTCPUDPSession* instance = (RTCPUDPSession*)p;
  if (instance == NULL) return;

  instance->removeLastReceivedSSRC();
}

extern "C" void RemoveSender(packet /*p*/) {
  // we don't yet recognize senders other than ourselves #####
}

