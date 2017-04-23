#ifndef _Z3_EXECUTOR_POOL_HPP_
#define _Z3_EXECUTOR_POOL_HPP_

#include "z3_executor.hpp"

namespace Z3 {

        class ExecutorPool
        {
        public:
                ExecutorPool();
                virtual ~ExecutorPool();

                int     Init(EV_QUEUE_T *pQueue, uint32_t nCount);
                void    Uninit();

        private:
                typedef std::list<Executor *> EXECUTORS_LIST;
                EXECUTORS_LIST m_lstExecutors;
        };
};

#endif