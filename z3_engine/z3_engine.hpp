#ifndef _Z3_ENGINE_HPP_
#define _Z3_ENGINE_HPP_

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#include <hash_map>

#include "z3_thread.hpp"
#include "z3_async_queue.hpp"

namespace Z3 {

        typedef AsyncQueue<Z3EV_ASYNCQUEUE_ITEM> EV_QUEUE_T;

        class Engine : public Thread
        {
        public:
                Engine(uint32_t nObjID = INVALID_OBJ_ID);

                //typedef enum _ACTION_TYPE
                //{
                //        OP_READ,
                //        OP_WRITE,
                //        OP_ADDRINFO,
                //        OP_LISTEN,
                //        OP_ACCEPT,
                //        OP_CONNECT,
                //        OP_TIMEOUT,
                //        OP_EXIT
                //} ACTION_TYPE;

                inline HANDLE
                        GetIOCP(void)   { return m_hIOCP; }

                inline EV_QUEUE_T*
                        GetAsyncQueue(void) { return m_pQueue; }

        protected:
                virtual ~Engine();

                virtual bool    OnThreadStart(void);
                virtual void    RunOnce();
                virtual void    OnThreadStop(void);

                bool    Dispatch(ev_id_t evID, LPZ3_EV_OVL pZ3Ovl);

        private:
                HANDLE          m_hIOCP;
                EV_QUEUE_T      *m_pQueue;
        };
};

#endif