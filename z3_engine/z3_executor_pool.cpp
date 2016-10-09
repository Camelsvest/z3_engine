#include "z3_common.hpp"
#include "z3_executor_pool.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

ExecutorPool::ExecutorPool()
{
}

ExecutorPool::~ExecutorPool()
{
        assert(m_lstExecutors.empty());
}

int ExecutorPool::Init(AsyncQueue *pQueue, uint32_t nCount)
{
        Executor *pExecutor;
        bool bOK;

        while (nCount > 0)
        {
                pExecutor = new Executor(pQueue, Z3_EXECUTOR_ID);
                assert(pExecutor);

                m_lstExecutors.push_back(pExecutor);
                Z3_OBJ_ADDREF(pExecutor);
                bOK = pExecutor->Start();
                assert(bOK);

                --nCount;
        }

        return 0;
}

void ExecutorPool::Uninit()
{
        Executor *pExecutor;

        while (!m_lstExecutors.empty())
        {
                pExecutor = m_lstExecutors.front();
                m_lstExecutors.pop_front();

                pExecutor->Stop();
                Z3_OBJ_RELEASE(pExecutor);
        }
}