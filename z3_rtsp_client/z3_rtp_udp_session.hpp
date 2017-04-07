#ifndef _Z3_RTP_UDP_SESSION_HPP_
#define _Z3_RTP_UDP_SESSION_HPP_

#include "z3_udp_session.hpp"
#include "z3_sdp.hpp"
#include "z3_rtp_msg.hpp"
#include "z3_h264_frame.hpp"
#include "z3_rtp_reception_stats.hpp"

namespace Z3 {
        
        class RTPUDPParser : public ProtoParser
        {
        public:
                RTPUDPParser();

                virtual int Parse(const char *pBuf, unsigned int nLen, void *pData, Msg **pMsg /* output */);

        protected:
                virtual ~RTPUDPParser();

        private:
                RTPPacket       *m_pRTPPacket;
                H264Frame       *m_pH264Frame;

                bool            m_bFrameCompleted;
        };

        class RTPUDPSession : public UDPSession
        {
        public:
                RTPUDPSession(HANDLE hIOCP);

                /*int     Init(SDPMedia *pSDPMedia);*/

        protected:
                virtual ~RTPUDPSession();


                virtual ProtoParser*    GetProtoParser();
                virtual int             Dispatch(Msg *pMsg, void *pData);

        private:
                RTPUDPParser    *m_pParser;
        };
};

#endif