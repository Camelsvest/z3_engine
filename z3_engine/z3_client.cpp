#include "z3_common.hpp"
#include "z3_client.hpp"
#include "z3_iocp_obj.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

Client::Client(uint32_t nObjID)
        : Thread(nObjID)
        , m_pEngine(NULL)
        , m_bStarted(false)
        , m_pMsgQueue(NULL)
{
}

Client::~Client()
{
        assert(!m_bStarted);
        assert(m_pEngine == NULL);
        assert(m_pMsgQueue == NULL);
}

bool Client::OnThreadStart(void)
{
        bool            bOK;
        EV_QUEUE_T      *pQueue;
        SYSTEM_INFO     SysInfo;
        int             nError;

        TRACE_ENTER_FUNCTION;

        OnClientStart();

        m_pMsgQueue = new NOTIFY_QUEUE_T;
        Z3_OBJ_ADDREF(m_pMsgQueue);

        m_pEngine = new Engine(Z3_ENGINE_ID);
        assert(m_pEngine);
        Z3_OBJ_ADDREF(m_pEngine);

        bOK = m_pEngine->Start();
        assert(bOK);

        pQueue = m_pEngine->GetAsyncQueue();
        assert(pQueue);

        GetSystemInfo(&SysInfo);

        nError = m_ExecutorPool.Init(pQueue, /*SysInfo.dwNumberOfProcessors * THREAD_NUM_FACTOR_PER_CPU*/1);
        if (!nError)
                m_bStarted = true;

        TRACE_EXIT_FUNCTION;

        if (m_bStarted)
                return Thread::OnThreadStart();
        else
                return false;
}

void Client::OnThreadStop(void)
{
        assert(m_pEngine);

        m_ExecutorPool.Uninit();
        m_pEngine->Stop();
        m_bStarted = false;
        Z3_OBJ_RELEASE(m_pEngine);
        Z3_OBJ_RELEASE(m_pMsgQueue);

        OnClientStop();

        return Thread::OnThreadStop();
}

bool Client::Running()
{
        bool bRunning;

        Thread::Lock();
        bRunning = m_bStarted;
        Thread::Unlock();

        return bRunning;
}

HANDLE Client::GetIOCP()
{
        HANDLE hIOCP;

        Thread::Lock();
        hIOCP = m_pEngine->GetIOCP();
        Thread::Unlock();

        return hIOCP;
};

void Client::RunOnce()
{
        Z3EV_NOTIFY_ITEM item;
        bool bSucceed;
        IOCPObj *pObj;

        //TRACE_ENTER_FUNCTION;

        assert(m_pMsgQueue);
        bSucceed = m_pMsgQueue->WaitForEV(item, 100 /*INFINITE*/);
        if (bSucceed)
        {
                assert(item.id != EV_UNKNOWN);
                
                Thread::Lock();
                OnNotify(item.id, item.nErrorCode, item.data);
                Thread::Unlock();

                pObj = static_cast<IOCPObj *>(item.data);
                
                // Before pushing into queue, AddRef was executed on pObj. Now remove it
                Z3_OBJ_RELEASE(pObj);
        }

        //TRACE_EXIT_FUNCTION;
}