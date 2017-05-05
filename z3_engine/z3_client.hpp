#ifndef _Z3_CLIENT_HPP_
#define _Z3_CLIENT_HPP_

#include "z3_engine.hpp"
#include "z3_executor_pool.hpp"
#include "z3_session_owner.hpp"

namespace Z3 {

        class Client : public Thread
        {
                typedef AsyncQueue<Z3EV_NOTIFY_ITEM> NOTIFY_QUEUE_T;
        public:
                Client(uint32_t nObjID = INVALID_OBJ_ID);

                bool     Running();
                HANDLE   GetIOCP();
                inline NOTIFY_QUEUE_T* GetNotifyQueue(void) { return m_pMsgQueue; }

        protected:
                virtual ~Client();

                virtual bool    OnThreadStart(void);
                virtual void    RunOnce();
                virtual void    OnThreadStop(void);

                virtual void    OnClientStart(void) = 0;
                virtual void    OnClientStop(void) = 0;
                virtual void    OnNotify(ev_id_t evID, uint32_t nErrorCode, void *pData) = 0;

        private:
                Engine          *m_pEngine;
                ExecutorPool    m_ExecutorPool;
                bool            m_bStarted;

                NOTIFY_QUEUE_T  *m_pMsgQueue;

        };

};

#endif