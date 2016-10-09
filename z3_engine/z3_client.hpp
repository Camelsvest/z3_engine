#ifndef _Z3_CLIENT_HPP_
#define _Z3_CLIENT_HPP_

#include "z3_engine.hpp"
#include "z3_executor_pool.hpp"
#include "z3_session_owner.hpp"

namespace Z3 {

        class Client : public SessionOwner
        {
        public:
                Client(uint32_t nObjID = INVALID_OBJ_ID);

                int     Start();
                void    Stop();

                /*virtual int     AddSession(const char *pszHost, uint16_t nPort) = 0;*/
                inline bool     Running() { return m_bStarted; }
                inline HANDLE   GetIOCP() { return m_pEngine->GetIOCP(); };

        protected:
                virtual ~Client();

                virtual void    OnClientStart(void) = 0;
                virtual void    OnClientStop(void) = 0;

        private:
                Engine          *m_pEngine;
                ExecutorPool    m_ExecutorPool;
                bool            m_bStarted;
        };

};

#endif