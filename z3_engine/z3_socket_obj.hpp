#ifndef _Z3_SOCKET_OBJ_HPP_
#define _Z3_SOCKET_OBJ_HPP_

#include "z3_io_endpoint.hpp"

namespace Z3 {

        class SocketObj : public IOEndpoint
        {
        public:
                SocketObj(HANDLE hIOCP, uint32_t nObjID);

                int     AsyncConnect(SOCKET hSocket, SOCKADDR_IN *pTarget, uint32_t nMillseconds);
                int     AsyncTCPRead(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf);
                int     AsyncTCPWrite(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf);

                int     AsyncUDPRead(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf, SOCKADDR *pSockAddr, int *pnAddrSize);
                int     AsyncUDPWrite(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf);

        protected:
                virtual ~SocketObj();

        private:
                SOCKET  m_hSocket;
        };
};

#endif
