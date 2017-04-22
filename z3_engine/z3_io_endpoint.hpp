#ifndef _Z3_IO_ENDPOINT_HPP_
#define _Z3_IO_ENDPOINT_HPP_

#include "z3_iocp_obj.hpp"

namespace Z3 {

        class IOEndpoint : public IOCPObj
        {
        public:
                IOEndpoint(HANDLE hIOCP, uint32_t nObjID);

                inline void     SetFileHandle(HANDLE hFileHandle);
                virtual HANDLE  GetFileHandle();

                virtual LPZ3_EV_OVL     AllocZ3Ovl(ev_id_t evID, uint32_t millseconds);
                virtual void            FreeZ3Ovl(LPZ3_EV_OVL pOvl);

        protected:
                virtual ~IOEndpoint();

        private:
                HANDLE  m_hFileHandle;
        };
};

#endif
