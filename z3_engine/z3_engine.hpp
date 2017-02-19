#ifndef _Z3_ENGINE_HPP_
#define _Z3_ENGINE_HPP_

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#include <hash_map>

#include "z3_thread.hpp"
#include "z3_async_queue.hpp"

namespace Z3 {

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

                inline HANDLE           GetIOCP(void)   { return m_hIOCP; }
                inline AsyncQueue*      GetAsyncQueue(void) { return &m_Queue; }

        protected:
                virtual ~Engine();

                virtual bool    OnThreadStart(void);
                virtual void    RunOnce();
                virtual void    OnThreadStop(void);

                void    AddIntoPendingList(LPZ3_EV_OVL pOvl);
                void    RemoveFromPendingList(LPZ3_EV_OVL pOvl);
                void    RemoveFromPendingList(uint32_t handle);

                bool    Dispatch(ev_id_t evID, LPZ3_EV_OVL pZ3Ovl, bool bTimeout = false);

        private:
                HANDLE  m_hIOCP;

                typedef std::list<LPZ3_EV_OVL>              Z3OVL_LIST;
                typedef std::list<LPZ3_EV_OVL>::iterator    Z3OVL_LIST_ITERATOR;

                Z3OVL_LIST      m_lstPendingOvl;
                AsyncQueue      m_Queue;

                //typedef stdext::hash_multimap<uint32_t, LPZ3_EV_OVL>    Z3OVL_HASH;
                //typedef stdext::hash_multimap<uint32_t, LPZ3_EV_OVL>::iterator Z3OVL_HASH_ITERATOR;

                //Z3OVL_HASH      m_hashPendingOvl;
        };
};

#endif