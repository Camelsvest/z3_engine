#ifndef _Z3_IOCP_OBJ_HPP_
#define _Z3_IOCP_OBJ_HPP_

#include "z3_timer_obj.hpp"
#include "z3_ev.hpp"

namespace Z3 {

        class IOCPObj : public TimerObj
        {
                friend class Engine;
                friend class Executor;
        public:
                IOCPObj(HANDLE hIOCP, uint32_t nObjID);
                
                int     SocketAsyncConnect(SOCKET hSocket, SOCKADDR_IN *pTarget, uint32_t nMillseconds);
                int     SocketAsyncTCPRead(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf);
                int     SocketAsyncTCPWrite(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf);

                int     SocketAsyncUDPRead(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf, SOCKADDR *pSockAddr, int *pnAddrSize);
                int     SocketAsyncUDPWrite(SOCKET hSocket, uint32_t nMillseconds, WSABUF *pwsaBuf);
              
                int Start(void);

                virtual int     Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes, bool bExpired = false) = 0;
                virtual void    OnTimer(void *pData);

        protected:
                virtual ~IOCPObj();

                //int     AddTimer(uint32_t nTimerID, uint32_t nMillseconds);
                //int     RemoveTimer(uint32_t nTimerID);

        private:
                LPZ3_EV_OVL     AllocZ3Ovl(HANDLE hFileHandle, ev_id_t evID, uint32_t millseconds);
                void            FreeZ3Ovl(LPZ3_EV_OVL pOvl);

                inline void     AddIntoPendingList(LPZ3_EV_OVL pOvl);

        protected:
                HANDLE  m_hIOCP;

        private:
                typedef std::list<LPZ3_EV_OVL>              Z3OVL_LIST;
                typedef std::list<LPZ3_EV_OVL>::iterator    Z3OVL_LIST_ITERATOR;

                Z3OVL_LIST      m_lstPendingZ3Ovl;
        };
};
#endif