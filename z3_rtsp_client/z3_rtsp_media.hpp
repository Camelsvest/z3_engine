#ifndef _Z3_RTSP_MEDIA_HPP_
#define _Z3_RTSP_MEDIA_HPP_

#include <list>
#include "z3_sdp.hpp"

namespace Z3 {

        enum MEDIA_TYPE
        {
                UNKNOWN_TYPE  = 0,
	        VIDEO_TYPE,
	        AUDIO_TYPE,
	        OTHER_TYPE
        };

        enum TRANSFER_MODE
        {
                UNKNOWN_MODE = 0,
                RTP_OVER_RTSP,
                RTP_OVER_UDP,
                RTP_OVER_TCP
        };

        class RTPUDPSession;
        class RTCPUDPSession;
        class RtspSession;

        class MediaSession : public MemoryObject
        {
        public:
                MediaSession(HANDLE hIOCP);
                virtual ~MediaSession();

                int     Init(SDPMedia *pSDPMedia);

                void    GetPort(uint16_t *pRTPPort, uint16_t *pRTCPPort);
                
                inline int      GetTransferMode() { return m_nTransferMode; }

        protected:
                HANDLE          m_hIOCP;
                uint16_t        m_nRTPPort;
                uint16_t        m_nRTCPPort;
                RTPUDPSession   *m_pRTPSession;
                RTCPUDPSession  *m_pRTCPSession;

                int             m_nMediaType;
                int             m_nTransferMode;
        };

        class RtspMedia : public MemoryObject
        {
        public:
                RtspMedia();
                virtual ~RtspMedia();

                bool    Init(char               *pszContextBase,
                             unsigned int       nTracks,
                             unsigned int       nRequestTrack,
                             SDPMessage         *pSDP);

                MediaSession *          CreateMediaSession(SDPMedia *pMedia, HANDLE hIOCP);
                inline char *           GetContentBase() { return m_pszContentBase; }
                inline SDPMessage *     GetSDPMessage() { return m_pSDP; }
                inline unsigned int     GetRequestTrack() { return m_nRequestTrack; }
                inline void             SetRequestTrack(unsigned int nValue) { m_nRequestTrack = nValue; }
                inline unsigned int     GetTrackCount() { return m_nTrackCount; }

        protected:
                bool                    AddMediaSession(MediaSession *pSession);

        private:
                char                            *m_pszContentBase;
                std::list<MediaSession *>       *m_pMediaSessions;
                SDPMessage                      *m_pSDP;
                unsigned int                    m_nTrackCount;
                unsigned int                    m_nRequestTrack;
        };

};

#endif