#ifndef _Z3_RTSP_CLIENT_HPP_
#define _Z3_RTSP_CLIENT_HPP_

#include "z3_client.hpp"
#include "z3_rtsp_ids.hpp"
#include "z3_rtsp_session.hpp"

namespace Z3 {

        class RtspClient : public Client
        {
        public:
                RtspClient(uint32_t nObjID = RTSP_CLIENT_ID);

                int     AddSession(const char *pszUrl);

                /*virtual int     AddSession(const char *pszHost, uint16_t nPort);*/
                virtual void    OnNotify(ev_id_t evID, uint32_t nErrorCode, void *pData);

        protected:
                virtual ~RtspClient();

                virtual void    OnClientStart(void);
                virtual void    OnClientStop(void);

                void    RemoveSession(RtspSession *pSession);

        private:
                typedef std::list<RtspSession *> RTSP_SESSION_LIST;
                RTSP_SESSION_LIST m_lstSessions;
        };
};

#endif