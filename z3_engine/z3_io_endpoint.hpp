#ifndef _Z3_IO_ENDPOINT_HPP_
#define _Z3_IO_ENDPOINT_HPP_

#include "z3_iocp_obj.hpp"

namespace Z3 {

        class IOEndpoint : public IOCPObj
        {
                friend class Engine;
                friend class Executor;
        public:
                IOEndpoint(HANDLE hIOCP, uint32_t nObjID);

                void            SetFileHandle(HANDLE hFileHandle);
                virtual HANDLE  GetFileHandle();

                virtual LPZ3_EV_OVL     AllocZ3Ovl(ev_id_t evID, uint32_t millseconds);
                virtual void            FreeZ3Ovl(LPZ3_EV_OVL pOvl);
                virtual void            OnTimer(ev_id_t evID, void *pData) = 0;

                int     CancelIO(LPZ3_EV_OVL pOvl);
                int     CancelTimer(LPZ3_EV_OVL pOvl);

                virtual int     Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes);
        protected:
                virtual ~IOEndpoint();

                virtual int     OnEvCompleted(ev_id_t evID, uint32_t nStatusCode, uint32_t nBytes) = 0;

        private:
                void            OnTimer(void *pData);

                HANDLE  m_hFileHandle;
        };
};

#endif
