#ifndef _Z3_UDP_SESSION_HPP_
#define _Z3_UDP_SESSION_HPP_

#include "z3_iocp_obj.hpp"
#include "z3_proto_parser.hpp"

namespace Z3 {

        class UDPSession : public IOCPObj
        {
        public:
                UDPSession(HANDLE hIOCP, uint32_t nObjID = INVALID_OBJ_ID);

                int	Init(uint16_t nPort);
                int     StartRead(uint32_t nTimeout /*Millseconds*/);

                inline SOCKET   GetSocket() { return m_hSocket; }
                inline char*    GetRecvBuffer() { return m_pUdpRecvBuf; }
                inline uint32_t GetRecvBufSize() { return m_nUdpRecvBufSize; }

                virtual int     Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes, bool bExpired);

        protected:
                virtual ~UDPSession();

                virtual int     OnEvRead(uint32_t nErrorCode, uint32_t nBytes, bool bExpired);
                virtual int     OnEvWrite(uint32_t nErrorCode, uint32_t nBytes, bool bExpired);

                virtual ProtoParser*    GetProtoParser() = 0;
                virtual int             Dispatch(Msg *pMsg, void *pData) = 0;

        private:
                SOCKET          m_hSocket;
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