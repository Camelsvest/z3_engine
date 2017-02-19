#ifndef _Z3_CONNECTOR_HPP_
#define _Z3_CONNECTOR_HPP_

#include "z3_iocp_obj.hpp"
#include "z3_msg.hpp"
#include "z3_proto_parser.hpp"

namespace Z3 {

        class Connector : public IOCPObj
        {
        public:
                Connector(HANDLE hIOCP, uint32_t nObjID = INVALID_OBJ_ID);

                bool            SetDestination(const char *pszHost, uint16_t nPort);
                inline SOCKET   GetSocket() { return m_hSocket; }

                inline char*            GetRecvBuffer() { return m_pRecvBuf; }
                inline unsigned int     GetRecvBufSize() { return m_nRecvBufSize; }

                virtual int     Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes, bool bExpired = false);

        protected:
                typedef enum _CONNECTOR_STATE
                {
                        CONN_UNCONNECTED,
                        CONN_CONNECTING,
                        CONN_CONNECTED
                } CONNECTOR_STATE;

                virtual ~Connector();

                int     Connect(uint32_t nTimeout /*Millseconds*/);
                virtual int     OnConnect(uint32_t nErrorCode, bool bExpired) = 0;
                virtual int     OnEvCompleted(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes, bool bExpired);
                virtual int     OnEvRead(uint32_t nErrorCode, uint32_t nBytes, bool bExpired);
                virtual int     OnEvWrite(uint32_t nErrorCode, uint32_t nBytes, bool bExpired);

                virtual ProtoParser*    GetProtoParser() = 0;
                virtual int     Dispatch(Msg *pMsg, void *pData) = 0;

                PADDRINFOEX     InterpretDNS();
                int             WriteMsg(Msg *pMsg);
                int             WriteMsg(const char *pBuf, uint32_t nSize);

                int             StartRead(uint32_t nTimeout /*Millseconds*/);

        private:
                SOCKET          m_hSocket;
                CONNECTOR_STATE m_ConnState;

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