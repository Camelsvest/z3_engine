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

                int     CancelIO(LPZ3_EV_OVL pOvl);
                int     CancelTimer(LPZ3_EV_OVL pOvl);

                virtual int     Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes);
                virtual void    OnTimer(void *pData);

        protected:
                virtual ~IOEndpoint();

                virtual int     OnEvCompleted(ev_id_t evID, uint32_t nStatusCode, uint32_t nBytes) = 0;
                virtual int     OnStart() = 0;
                virtual int     OnStop() = 0;

        private:
                HANDLE  m_hFileHandle;
        };
};

#endif
