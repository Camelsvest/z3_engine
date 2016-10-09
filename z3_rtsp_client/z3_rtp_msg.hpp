#ifndef _Z3_RTP_MSG_HPP_
#define _Z3_RTP_MSG_HPP_

#include "z3_msg.hpp"
#include "z3_rtp.hpp"

namespace Z3 {

        class RTPPacket : public MemoryObject
        {
        public:
                RTPPacket();
                virtual ~RTPPacket();

                void            InitPacket(char *pBuf, size_t nBufSize);
                unsigned char*  GetPayload();
                uint32_t        GetPayloadSize();
                unsigned long   GetTimestamp();
                bool            IsMarker();

        private:
                char            *m_pPacket;
                uint32_t        m_nPacketSize;
        };

        class RtpMsg : public Msg
        {
        public:
                RtpMsg();

                virtual unsigned int    ProtoID();
                int     FillBuffer(const char *pRtpPacket, unsigned int nPktSize);

                unsigned char*  GetPayload();
                unsigned int    GetPayloadSize();

        protected:
                virtual ~RtpMsg();

                virtual bool            ToStringImpl(char **ppbuf, unsigned int *pnSize);

        private:
                char            *m_pPacket;
                uint16_t        m_nPacketSize;
        };
        
};

#endif