#ifndef _Z3_EXECUTOR_HPP_
#define _Z3_EXECUTOR_HPP_

#include "z3_thread.hpp"
#include "z3_engine.hpp"

namespace Z3 {

        class Executor : public Thread
        {
        public:
                Executor(EV_QUEUE_T *pQueue, uint32_t nObjID = INVALID_OBJ_ID);

        protected:
                virtual ~Executor();

                virtual bool    OnThreadStart(void);
                virtual void    RunOnce();
                virtual void    OnThreadStop(void);

        private:
                EV_QUEUE_T      *m_pQueue;
        };
};

#endif