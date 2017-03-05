#include <assert.h>
#include "z3_common.hpp"
#include "z3_timer.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

Timer* Timer::m_pInstance = NULL;
HANDLE Timer::m_hMutex = NULL;
HANDLE Timer::m_hTimerQueue = NULL;

Timer::Timer()
        :MemoryObject()
{
        assert(m_pInstance == NULL);
        assert(m_hMutex == NULL);

        m_hTimerQueue = ::CreateTimerQueue();
        assert(m_hTimerQueue);
}

Timer::~Timer()
{
        if (m_hTimerQueue)
                ::DeleteTimerQueueEx(m_hTimerQueue, NULL);
}

void Timer::TimerCallBack(PVOID lpParameter, BOOLEAN bTimerExpired)
{

}

Timer* Timer::Instance()
{
        DWORD   dwResult; 
        BOOL    bOK;
        
        if (m_hMutex == NULL)
        {
                m_hMutex = ::CreateMutex(NULL, FALSE, _T("Z3_TIMER_MUTEX"));
                assert(m_hMutex);
        }

        dwResult = ::WaitForSingleObject(m_hMutex, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);

        if (m_pInstance == NULL)
        {
                m_pInstance = new Timer();
        }

        bOK = ::ReleaseMutex(m_hMutex);
        assert(bOK);

        return m_pInstance;
}

void Timer::Destroy()
{
        DWORD dwResult;

        assert(m_hMutex != NULL);
        dwResult = ::WaitForSingleObject(m_hMutex, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);

        if (m_pInstance)
                delete m_pInstance;

        ::CloseHandle(m_hMutex);
}

HANDLE Timer::AddTimer(uint32_t nObjID, uint32_t millseconds, bool bRepeat)
{
        BOOL            bOK;
        HANDLE          hTimer;
        uint32_t        period;

        assert(m_hTimerQueue);

        period = bRepeat ? millseconds : 0;

        bOK = ::CreateTimerQueueTimer(&hTimer, m_hTimerQueue, Timer::TimerCallBack,
                (PVOID)nObjID, millseconds, period, WT_EXECUTEDEFAULT);

        return bOK ? hTimer : NULL;
}

bool Timer::DeleteTimer(uint32_t nObjID, HANDLE hTimer)
{
        BOOL    bOK;

        bOK = ::DeleteTimerQueueTimer(m_hTimerQueue, hTimer, NULL);

        return bOK ? true : false;
}
