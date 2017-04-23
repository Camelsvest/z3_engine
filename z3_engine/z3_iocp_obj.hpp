#ifndef _Z3_IOCP_OBJ_HPP_
#define _Z3_IOCP_OBJ_HPP_

#include "z3_timer_obj.hpp"
#include "z3_ev.hpp"
#include "z3_session_owner.hpp"

namespace Z3 {

        class IOCPObj : public TimerObj
        {
        public:
                IOCPObj(HANDLE hIOCP, uint32_t nObjID);
                           
                int     Start(SessionOwner *pOwner);
                int     Stop(void);

                SessionOwner* GetOwner() { return m_pOwner; }

                virtual LPZ3_EV_OVL     AllocZ3Ovl(ev_id_t evID, uint32_t millseconds) = 0;
                virtual void            FreeZ3Ovl(LPZ3_EV_OVL pOvl) = 0;

        protected:
                virtual ~IOCPObj();

                inline int      PostCompletionStatus(LPOVERLAPPED pOvl);

        protected:
                HANDLE          m_hIOCP;
                SessionOwner    *m_pOwner;

        };
};
#endif