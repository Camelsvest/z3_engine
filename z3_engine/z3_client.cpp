#include "z3_common.hpp"
#include "z3_client.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

Client::Client(uint32_t nObjID)
        : SessionOwner(nObjID)
        , m_pEngine(NULL)
        , m_bStarted(false)
{
}

Client::~Client()
{
        
}

int Client::Start()
{
        bool            bOK;
        EV_QUEUE_T      *pQueue;
        SYSTEM_INFO     SysInfo;
        int             nError;

        TRACE_ENTER_FUNCTION;

        OnClientStart();

        m_pEngine = new Engine(Z3_ENGINE_ID);
        assert(m_pEngine);
        Z3_OBJ_ADDREF(m_pEngine);
        
        bOK = m_pEngine->Start();
        assert(bOK);

        pQueue = m_pEngine->GetAsyncQueue();
        assert(pQueue);

        GetSystemInfo(&SysInfo);
        
        nError = m_ExecutorPool.Init(pQueue, SysInfo.dwNumberOfProcessors * THREAD_NUM_FACTOR_PER_CPU);
        if (!nError)
                m_bStarted = true;

        TRACE_EXIT_FUNCTION;

        return nError;
}

void Client::Stop()
{
        assert(m_pEngine);

        m_ExecutorPool.Uninit();
        m_pEngine->Stop();        
        m_bStarted = false;
        Z3_OBJ_RELEASE(m_pEngine);

        OnClientStop();

        return;
}

