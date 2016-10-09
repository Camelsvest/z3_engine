#include "z3_common.hpp"
#include "z3_thread.hpp"

using namespace Z3;

Thread::Thread(uint32_t nObjID)
        : AsyncObj(nObjID)
        , m_hThread(NULL)
        , m_nThreadID(0)
        , m_hMutex(NULL)
        , m_hEvRunning(NULL)
        , m_bQuit(false)
{
}

Thread::~Thread()
{
        assert(m_hThread == NULL);
        assert(m_nThreadID == 0);
        assert(m_hMutex == NULL);
        assert(m_bQuit);
}

bool Thread::OnThreadStart(void)
{
        assert(m_hEvRunning);
        return ::SetEvent(m_hEvRunning) ? true : false;
}

void Thread::OnThreadStop(void)
{
        TRACE_DETAIL("Thread(0x%p) is quitting\r\n", this);
}

unsigned __stdcall Thread::ThreadFunc(void *pArgs)
{
        Thread  *pThis;
        DWORD   dwResult;
        bool    bQuit;

        pThis = static_cast<Thread *>(pArgs);

        if (!pThis->OnThreadStart())
                return -1;

        do
        {
                dwResult = ::WaitForSingleObject(pThis->m_hMutex, INFINITE);
                assert(dwResult == WAIT_OBJECT_0);

                bQuit = pThis->m_bQuit;

                ::ReleaseMutex(pThis->m_hMutex);

                if (!bQuit)
                        pThis->RunOnce();
        }
        while (!bQuit);

        pThis->OnThreadStop();

        return 0;
}

bool Thread::Start(void)
{
        DWORD dwResult;

        assert(m_hMutex == NULL);        
        m_hMutex = ::CreateMutexA(NULL, FALSE, NULL);
        assert(m_hMutex != NULL);
        
        assert(m_hEvRunning == NULL);
        m_hEvRunning = ::CreateEventA(NULL, FALSE, FALSE, NULL);
        assert(m_hEvRunning != NULL);

        assert(m_hThread == NULL);
        assert(m_nThreadID == 0);
        m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, this, 0, &m_nThreadID);
        
        dwResult = WaitForSingleObject(m_hEvRunning, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);

        return (m_hThread != NULL);
}

void Thread::Stop(void)
{
        DWORD   dwResult;

        assert(m_hMutex);
        dwResult = ::WaitForSingleObject(m_hMutex, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);
        m_bQuit = TRUE;
        ::ReleaseMutex(m_hMutex);

        TRACE_DETAIL("Inform thread(0x%p) to quit\r\n", this);

        assert(m_hThread != NULL);
        dwResult = ::WaitForSingleObject(m_hThread, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);

        Z3_CLOSE_HANDLE(m_hThread);
        m_nThreadID = 0;

        Z3_CLOSE_HANDLE(m_hMutex);
        Z3_CLOSE_HANDLE(m_hEvRunning);
        
        return;
}