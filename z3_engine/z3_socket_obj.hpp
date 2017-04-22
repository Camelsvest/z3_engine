#ifndef _Z3_SOCKET_OBJ_HPP_
#define _Z3_SOCKET_OBJ_HPP_

#include "z3_io_endpoint.hpp"

namespace Z3 {

        class SocketObj : public IOEndpoint
        {
        public:
                typedef enum _SOCKET_TYPE
                {
                        TCP_SOCK,
                        UDP_SOCK
                } SOCKET_TYPE;

                SocketObj(HANDLE hIOCP, uint32_t nObjID);

                int      Init(SOCKET_TYPE type);
                int      Close(void);
                int      Bind(uint16_t nPort);

                inline SOCKET   GetSocket() { return m_hSocket; }

                int     AsyncConnect(SOCKADDR_IN *pTarget, uint32_t nMillseconds);
                int     AsyncTCPRead(uint32_t nMillseconds, WSABUF *pwsaBuf);
                int     AsyncTCPWrite(uint32_t nMillseconds, WSABUF *pwsaBuf);

                int     AsyncUDPRead(uint32_t nMillseconds, WSABUF *pwsaBuf, SOCKADDR *pSockAddr, int *pnAddrSize);
                int     AsyncUDPWrite(uint32_t nMillseconds, WSABUF *pwsaBuf);

        protected:
                virtual ~SocketObj();

        private:
                SOCKET  m_hSocket;
        };
};

#endif
