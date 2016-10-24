#include "z3_common.hpp"
#include "z3_rtp_reception_stats.hpp"

extern "C" int gettimeofday(struct timeval* tp, int *tz);

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

RTPReceptionStats::RTPReceptionStats()
        : m_nLastReceivedSR_NTP(0)
        , m_nLastRTPTimestamp(0)
        , m_bSynchronized(false)
{
        memset(&m_SyncTime, 0, sizeof(m_SyncTime));
        memset(&m_nLastReceivedSR_time, 0, sizeof(m_nLastReceivedSR_time));
}

RTPReceptionStats::~RTPReceptionStats()
{
}

void RTPReceptionStats::ProcessIncomingSR(SR *pRTCPSR)
{
        ULARGE_INTEGER  li;
        double          microseconds;

        m_nLastReceivedSR_NTP = pRTCPSR->GetNTPTimestamp();
        m_nLastRTPTimestamp = pRTCPSR->GetRTPTimestamp();

        gettimeofday(&m_nLastReceivedSR_time, NULL);

        li.QuadPart = m_nLastReceivedSR_NTP;
        m_SyncTime.tv_sec = li.HighPart - 0x83AA7E80; // 1/1/1900 -> 1/1/1970
        microseconds = (li.LowPart * 15625.0) / 0x04000000; // 10^6 / 2^32
        m_SyncTime.tv_usec = (unsigned long)(microseconds + 0.5);
}

RTPReceptionStatsDB::RTPReceptionStatsDB()
{
}

RTPReceptionStatsDB::~RTPReceptionStatsDB()
{
        std::map<uint32_t, RTPReceptionStats *>::iterator itera;
        RTPReceptionStats *pStats;

        for (itera = m_mapReceptionStats.begin(); itera != m_mapReceptionStats.end(); itera++)
        {
                pStats = itera->second;

                delete pStats;
        }
}

void RTPReceptionStatsDB::Add(uint32_t nSSRC, RTPReceptionStats *pStats)
{
        typedef std::pair<uint32_t, RTPReceptionStats *> RECEPTIONS_PAIR;

        m_mapReceptionStats.insert(RECEPTIONS_PAIR(nSSRC, pStats));
}

RTPReceptionStats* RTPReceptionStatsDB::Lookup(uint32_t nSSRC) const
{
        std::map<uint32_t, RTPReceptionStats *>::const_iterator itera;
        RTPReceptionStats *pStats = NULL;

        itera = m_mapReceptionStats.find(nSSRC);
        if (itera != m_mapReceptionStats.end())
                pStats = itera->second;

        return pStats;
}

void RTPReceptionStatsDB::ProcessIncomingSR(SR *pRTCPSR)
{
        uint32_t                nSSRC;
        RTPReceptionStats       *pStats;

        nSSRC = pRTCPSR->GetSSRC();
        pStats = Lookup(nSSRC);
        if (pStats == NULL)
        {
                pStats = new RTPReceptionStats;
                assert(pStats);
                Add(nSSRC, pStats);
        }

        pStats->ProcessIncomingSR(pRTCPSR);
}