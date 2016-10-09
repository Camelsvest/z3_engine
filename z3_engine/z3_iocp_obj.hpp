#ifndef _Z3_IOCP_OBJ_HPP_
#define _Z3_IOCP_OBJ_HPP_

#include "z3_obj.hpp"
#include "z3_ev.hpp"

namespace Z3 {

        class IOCPObj : public AsyncObj
        {
        public:
                IOCPObj(HANDLE hIOCP, uint32_t nObjID);

                LPZ3OVL AllocZ3Ovl(HANDLE hFileHandle, ev_id_t evID, uint32_t millseconds);
                void    FreeZ3Ovl(LPZ3OVL pOvl);

                int Start(void);

                virtual int     Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes) = 0;

        protected:
                virtual ~IOCPObj();

        protected:
                HANDLE  m_hIOCP;

        private:
                typedef std::list<LPZ3OVL>              Z3OVL_LIST;
                typedef std::list<LPZ3OVL>::iterator    Z3OVL_LIST_ITERATOR;

                Z3OVL_LIST      m_lstIdle;
        };
};
#endif