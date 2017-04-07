#ifndef _Z3_RTCP_UDP_SESSION_HPP_
#define _Z3_RTCP_UDP_SESSION_HPP_

#include "z3_common.hpp"
#include "z3_udp_session.hpp"
#include "z3_rtcp_msg.hpp"
#include "z3_rtp_reception_stats.hpp"

namespace Z3 {

        class RTCPUDPParser : public ProtoParser
        {
        public:
                RTCPUDPParser();

                virtual int Parse(const char *pBuf, unsigned int nLen, void *pData, Msg **pMsg /* output */);

        protected:
                virtual ~RTCPUDPParser();

        private:
        };

        class RTCPUDPSession;

        class RTCPMemberDB : public MemoryObject
        {
        public:
                RTCPMemberDB(RTCPUDPSession& instance);
                virtual ~RTCPMemberDB();

                bool IsMember(uint32_t ssrc) const;
                bool NoteMembership(uint32_t ssrc, uint32_t curTimeCount);
                bool Remove(uint32_t ssrc);

                inline uint32_t NumMembers() const { return m_nNumMembers; }

                void ReapOldMembers(uint32_t threshold);

        private:
                RTCPUDPSession&       m_RTCPSession;
                uint32_t              m_nNumMembers;
                std::map<uint32_t, uint32_t>  m_map;
        };

        class RTCPUDPSession : public UDPSession
        {
        public:
                RTCPUDPSession(HANDLE hIOCP);

                int     CheckNewSSRC();
                void    RemoveSSRC(uint32_t nSSRC, bool bRemoveStats);
                uint32_t        NumMembers();

                void    Schedule(double dbNextTime);

        protected:
                virtual ~RTCPUDPSession();

                virtual ProtoParser*    GetProtoParser();
                virtual int             Dispatch(Msg *pMsg, void *pData);

                void    OnReceive(int nPayloadType, uint32_t nTotalBytes, uint32_t nSSRC);

        private:
                RTCPUDPParser           *m_pParser;
                RTPReceptionStatsDB     *m_pStatsDB;
                RTCPMemberDB            *m_pKnownMembers;

                uint32_t                m_nLastReceivedSSRC;
                uint32_t                m_nLastReceivedSize;
                uint8_t                 m_nPayloadType;

                uint32_t                m_nOutgoingReportCount;
                int                     m_nPrevNumMembers;
                double                  m_dbAveRTCPSize;
                double                  m_dbPrevReportTime;
                double                  m_dbNextReportTime;
        };
};

#endif
