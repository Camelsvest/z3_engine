#ifndef _Z3_UDP_SESSION_HPP_
#define _Z3_UDP_SESSION_HPP_

#include "z3_socket_obj.hpp"
#include "z3_proto_parser.hpp"

namespace Z3 {

        class UDPSession : public SocketObj
        {
        public:
                UDPSession(HANDLE hIOCP, uint32_t nObjID = INVALID_OBJ_ID);

                int	Init(uint16_t nPort);
                int     StartRead(uint32_t nTimeout /*Millseconds*/);

                inline char*    GetRecvBuffer() { return m_pUdpRecvBuf; }
                inline uint32_t GetRecvBufSize() { return m_nUdpRecvBufSize; }

                virtual int     Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes);

        protected:
                virtual ~UDPSession();

                virtual int     OnEvRead(uint32_t nErrorCode, uint32_t nBytes);
                virtual int     OnEvWrite(uint32_t nErrorCode, uint32_t nBytes);

                virtual ProtoParser*    GetProtoParser() = 0;
                virtual int             Dispatch(Msg *pMsg, void *pData) = 0;

        private:
                uint16_t        m_nPort;
                HANDLE          m_hIOCP;

                SOCKADDR_IN     m_addrFrom;
                int             m_nAddrFromSize;
                
                char            *m_pUdpRecvBuf;
                uint32_t        m_nUdpRecvBufSize;
                WSABUF          m_wsaRecvBuf;
        };
};

#endif