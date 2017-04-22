#include "z3_common.hpp"
#include "z3_timer_obj.hpp"

using namespace Z3;

TimerEngine*    TimerEngine::m_pInstance = NULL;
HANDLE          TimerEngine::m_hMutex = NULL;
HANDLE          TimerEngine::m_hTimerQueue = NULL;
uint32_t        TimerEngine::m_nTimerIDIndex = 10000;

TimerEngine::TimerEngine()
        :RefObj()
{
        assert(m_pInstance == NULL);
        assert(m_hMutex != NULL);

        m_hTimerQueue = ::CreateTimerQueue();
        assert(m_hTimerQueue);
}

TimerEngine::~TimerEngine()
{
	HANDLE	hCompletionEvent;
	DWORD	dwResult;

	if (m_hTimerQueue)
	{
		hCompletionEvent = ::CreateEvent(NULL, FALSE, NULL, NULL);
		assert(hCompletionEvent);

		if (::DeleteTimerQueueEx(m_hTimerQueue, hCompletionEvent) != 0)
		{
			dwResult = ::WaitForSingleObject(hCompletionEvent, INFINITE);
			assert(dwResult == WAIT_OBJECT_0);
		}

		::CloseHandle(hCompletionEvent);
	}
}

void TimerEngine::TimerCallBack(PVOID lpParameter, BOOLEAN bTimerExpired)
{
        LPZ3_EV_OVL     pOvl;
        TimerObj        *pTimerObj;

        pOvl = (LPZ3_EV_OVL)lpParameter;
        assert(pOvl);

        pTimerObj = static_cast<TimerObj *>(pOvl->data);
        assert(pTimerObj);

        pTimerObj->Lock();
        pTimerObj->OnTimer(pOvl);
        pTimerObj->Unlock();

        return;
}

TimerEngine* TimerEngine::Instance()
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
                m_pInstance = new TimerEngine();
        }
        m_pInstance->AddRef();

        bOK = ::ReleaseMutex(m_hMutex);
        assert(bOK);

        return m_pInstance;
}

void TimerEngine::Destroy()
{
        DWORD dwResult;

        assert(m_hMutex != NULL);
        dwResult = ::WaitForSingleObject(m_hMutex, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);

        if (m_pInstance)
                m_pInstance->Release(); // delete m_pInstance;

        ::CloseHandle(m_hMutex);
}

uint32_t TimerEngine::CreateTimerID()
{
        uint32_t nTimerID;
        DWORD dwResult;

        assert(m_hMutex != NULL);
        dwResult = ::WaitForSingleObject(m_hMutex, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);

        nTimerID = ++m_nTimerIDIndex;

        ::CloseHandle(m_hMutex);

        return nTimerID;
}

HANDLE TimerEngine::AddTimer(LPHANDLE phTimer, PVOID lpParameter, uint32_t millseconds, bool bRepeat)
{
        BOOL            bOK;
        uint32_t        period;

        assert(m_hTimerQueue != INVALID_HANDLE_VALUE);
        assert(phTimer != NULL);

        period = bRepeat ? millseconds : 0;

        bOK = ::CreateTimerQueueTimer(phTimer, m_hTimerQueue, TimerEngine::TimerCallBack,
                lpParameter, millseconds, period, WT_EXECUTEDEFAULT);

        return bOK ? *phTimer : NULL;
}

bool TimerEngine::DeleteTimer(HANDLE hTimer)
{
        BOOL    bOK, bRepeat = TRUE;
        HANDLE  hCompletionEvent;
        DWORD   dwResult;

        assert(hTimer != NULL);

        hCompletionEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hCompletionEvent == NULL)
        {
                TRACE_ERROR("Failed to create completion event when trying to delete a timer:0x%X", hTimer);
                return false;
        }

        do
        {
                bOK = ::DeleteTimerQueueTimer(m_hTimerQueue, hTimer, hCompletionEvent);
                if (bOK)
                        bRepeat = false;
                else
                {
                        dwResult = ::GetLastError();
                        if (dwResult == ERROR_IO_PENDING)
                                bRepeat = false;
                        else
                        {
                                TRACE_WARN("Failed to delete Timer(0x%X) from timer queue(0x%X), retry after 10 millseconds\r\n");
                                ::Sleep(10);
                        }                                
                }

        } while (bRepeat);
        
        dwResult = ::WaitForSingleObject(hCompletionEvent, INFINITE);
        ::CloseHandle(hCompletionEvent);

        if (dwResult == WAIT_OBJECT_0)
        {
                return true;
        }

        return false;
}

TimerObj::TimerObj(uint32_t nObjID)
        : AsyncObj(nObjID)
        , m_pTimerEngine(NULL)
{
}

TimerObj::~TimerObj()
{
        Z3_OBJ_RELEASE(m_pTimerEngine);
}

bool TimerObj::AddTimer(LPHANDLE phTimer, void *pData, uint32_t millseconds, bool bRepeat/* = false*/)
{
        HANDLE  handle;

        if (m_pTimerEngine == NULL)
                m_pTimerEngine = TimerEngine::Instance();

        handle = m_pTimerEngine->AddTimer(phTimer, pData, millseconds, bRepeat);
        return (handle != INVALID_HANDLE_VALUE);
}

bool TimerObj::DeleteTimer(HANDLE hTimer)
{
        assert(hTimer != INVALID_HANDLE_VALUE);
        assert(m_pTimerEngine != NULL);
        return m_pTimerEngine->DeleteTimer(hTimer);
}

void TimerObj::OnTimer(void *pData)
{
        TRACE_WARN("Default Timer function is invokded. You should override function: %s\r\n", __FUNCTION__);
        return;
}
