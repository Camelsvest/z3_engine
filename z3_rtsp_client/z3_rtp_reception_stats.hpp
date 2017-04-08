#ifndef _Z3_RTP_RECEPTION_STATS_HPP_
#define _Z3_RTP_RECEPTION_STATS_HPP_

#include "z3_async_obj.hpp"
#include "z3_rtcp_msg.hpp"

namespace Z3 {

        class RTPReceptionStats : public MemoryObject
        {
        public:
                RTPReceptionStats();
                virtual ~RTPReceptionStats();

                void    ProcessIncomingSR(SR *pRTCPSR);

        private:
                uint64_t        m_nLastReceivedSR_NTP;
                struct timeval  m_nLastReceivedSR_time;
                uint32_t        m_nLastRTPTimestamp;
                struct timeval  m_SyncTime;
                bool            m_bSynchronized;
        };

        class RTPReceptionStatsDB : public MemoryObject
        {
        public:
                RTPReceptionStatsDB();
                virtual ~RTPReceptionStatsDB();

                void    Add(uint32_t nSSRC, RTPReceptionStats *pStats);
                RTPReceptionStats *     Lookup(uint32_t nSSRC) const;

                void    ProcessIncomingSR(SR *pRTCPSR);

        private:
                std::map<uint32_t, RTPReceptionStats *>  m_mapReceptionStats;
        };
};

#endif