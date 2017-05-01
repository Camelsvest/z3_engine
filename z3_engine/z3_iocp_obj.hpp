#ifndef _Z3_IOCP_OBJ_HPP_
#define _Z3_IOCP_OBJ_HPP_

#include "z3_timer_obj.hpp"
#include "z3_ev.hpp"
#include "z3_async_queue.hpp"

namespace Z3 {

        class IOCPObj : public TimerObj
        {
                typedef AsyncQueue<Z3EV_NOTIFY_ITEM> NOTIFY_QUEUE_T;
        public:
                IOCPObj(HANDLE hIOCP, uint32_t nObjID);
                           
                int     Start(NOTIFY_QUEUE_T *pNotifyQueue);
                int     Stop(void);

                bool    IsStarted();
                bool    IsStopped();

                virtual LPZ3_EV_OVL     AllocZ3Ovl(ev_id_t evID, uint32_t millseconds) = 0;
                virtual void            FreeZ3Ovl(LPZ3_EV_OVL pOvl) = 0;

                virtual int     Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes);

        protected:
                virtual ~IOCPObj();

                int     PostCompletionStatus(LPOVERLAPPED pOvl);
                void    Notify(ev_id_t evID, uint32_t nErrroCode);

                virtual int     OnStart();
                virtual int     OnStop();

        protected:
                HANDLE          m_hIOCP;
                NOTIFY_QUEUE_T  *m_pNotifyQueue;
                bool            m_bStarted;

        };
};
#endif