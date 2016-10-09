#ifndef _Z3_RTSP_SESSION_HPP_
#define _Z3_RTSP_SESSION_HPP_

#include "z3_rtsp_ids.hpp"
#include "z3_connector.hpp"
#include "z3_session_owner.hpp"
#include "z3_rtsp_def.hpp"
#include "z3_rtsp_msg.hpp"
#include "z3_rtsp_proto_parser.hpp"
#include "z3_rtsp_media.hpp"

namespace Z3 {

        class RtspSession : public Connector
        {
        public:
                RtspSession(SessionOwner *pOwner, HANDLE hIOCP, uint32_t nObjID = RTSP_SESSION_ID);

                enum _SessionState
                {
                        RTSP_SESSION_INIT = 0,
                        RTSP_SESSION_READY,
                        RTSP_SESSION_PLAYING,
                        RTSP_SESSION_RECORDING,

                        STATE_CONNECT_TIMEOUT
                };

                int     SetRequestUrl(const char *pszUrl);

        protected:
                virtual ~RtspSession();

                virtual int     OnConnect(uint32_t nErrorCode);
                virtual ProtoParser*    GetProtoParser();
                virtual int     Dispatch(Msg *pMsg, void *pData);

                int     InvokeStateMachine(RTSP_METHOD method, RtspMsg *pResponse, void *pData);
                int     SendOPTIONS();
                int     SendDESCRIBE(RtspMsg *pResponse);
                int     SendSETUP();
                int     SendPLAY();

                int     OnOptionsRespond(RtspMsg *pResponse, void *pData);
                int     OnDescribeRespond(RtspMsg *pResponse, void *pData);
                int     OnSetupRespond(RtspMsg *pResponse, void *pData);

        private:
                SessionOwner    *m_pOwner;
                char            *m_pszUrl;

                RTSP_METHOD     m_methodPrevious;
                int             m_nCSeqPrevious;
                char            *m_pszSessionIdPrevious;
                
                RtspMedia       *m_pRtspMedia;
                RtspProtoParser *m_pParser;

                uint32_t        m_nSessionState;
                char            *m_pszSessionId;
                int             m_nTimeout;     // seconds
        };
};

#endif