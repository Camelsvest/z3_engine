#ifndef _Z3_CONNECTOR_HPP_
#define _Z3_CONNECTOR_HPP_

#include "z3_socket_obj.hpp"
#include "z3_msg.hpp"
#include "z3_proto_parser.hpp"

namespace Z3 {

        class Connector : public SocketObj
        {
        public:
                Connector(HANDLE hIOCP, uint32_t nObjID = INVALID_OBJ_ID);

                bool            SetDestination(const char *pszHost, uint16_t nPort);

                inline char*            GetRecvBuffer() { return m_pRecvBuf; }
                inline unsigned int     GetRecvBufSize() { return m_nRecvBufSize; }

        protected:
                virtual ~Connector();

                virtual int     OnEvCompleted(ev_id_t evID, uint32_t nStatusCode, uint32_t nBytes);
                virtual int     OnStart();
                virtual int     OnStop();

                int     Connect(uint32_t nTimeout /*Millseconds*/);
                virtual int     OnConnect(uint32_t nErrorCode);
                virtual int     OnEvRead(uint32_t nErrorCode, uint32_t nBytes);
                virtual int     OnEvWrite(uint32_t nErrorCode, uint32_t nBytes);

                virtual ProtoParser*    GetProtoParser() = 0;
                virtual int     Dispatch(Msg *pMsg, void *pData) = 0;

                PADDRINFOEX     InterpretDNS();
                int             WriteMsg(Msg *pMsg);
                int             WriteMsg(const char *pBuf, uint32_t nSize);

                int             StartRead(uint32_t nTimeout /*Millseconds*/);

        private:
                char            *m_pRecvBuf;
                uint32_t        m_nRecvBufSize;

                char            *m_pSendBuf;
                uint32_t        m_nSendBufSize;

                WSABUF          m_wsaSendBuf;
                WSABUF          m_wsaRecvBuf;

                char            *m_pszHost;
                uint16_t        m_nPort;
        };

};

#endif