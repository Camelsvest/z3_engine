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
        assert(m_hMutex == NULL);

        m_hTimerQueue = ::CreateTimerQueue();
        assert(m_hTimerQueue);
}

TimerEngine::~TimerEngine()
{
        if (m_hTimerQueue)
                ::DeleteTimerQueueEx(m_hTimerQueue, NULL);
}

typedef struct _timer_ctx
{
        TimerObj *pObj;
        uint32_t nTimerID;
        void     *pData;
} timer_ctx_t;

void TimerEngine::TimerCallBack(PVOID lpParameter, BOOLEAN bTimerExpired)
{
        timer_ctx_t     *pCtx;
        TimerObj        *pTimerObj;

        pCtx = static_cast<timer_ctx_t *>(lpParameter);
        assert(pCtx);

        pTimerObj = pCtx->pObj;
        assert(pTimerObj);

        pTimerObj->OnTimer(pCtx->nTimerID, pCtx->pData);

        z3_free(pCtx);

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

HANDLE TimerEngine::AddTimer(PVOID lpParameter, uint32_t millseconds, bool bRepeat)
{
        BOOL            bOK;
        HANDLE          hTimer;
        uint32_t        period;

        assert(m_hTimerQueue);

        period = bRepeat ? millseconds : 0;

        bOK = ::CreateTimerQueueTimer(&hTimer, m_hTimerQueue, TimerEngine::TimerCallBack,
                lpParameter, millseconds, period, WT_EXECUTEDEFAULT);

        return bOK ? hTimer : NULL;
}

bool TimerEngine::DeleteTimer(uint32_t nObjID, HANDLE hTimer)
{
        BOOL    bOK;
        HANDLE  hCompletionEvent;
        DWORD   dwResult;

        assert(hTimer != NULL);

        hCompletionEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hCompletionEvent == NULL)
        {
                TRACE_ERROR("Failed to create completion event when trying to delete a timer:0x%X", hTimer);
                return false;
        }

        bOK = ::DeleteTimerQueueTimer(m_hTimerQueue, hTimer, hCompletionEvent);

        dwResult = ::WaitForSingleObject(hCompletionEvent, INFINITE);
        Z3_CLOSE_HANDLE(hCompletionEvent);

        if (dwResult == WAIT_OBJECT_0 && bOK)
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

bool TimerObj::AddTimer(uint32_t nTimerID, void *pData, uint32_t millseconds, bool bRepeat/* = false*/)
{
        timer_ctx_t     *pCtx;
        HANDLE          handle;

        if (m_mapTimerHandle.find(nTimerID) != m_mapTimerHandle.end())
                return false; // nTimerID is already exist

        if (m_pTimerEngine == NULL)
                m_pTimerEngine = TimerEngine::Instance();

        pCtx = (timer_ctx_t *)z3_calloc(sizeof(timer_ctx_t), 1);
        if (pCtx != NULL)
        {
                pCtx->pObj = this;
                pCtx->nTimerID = nTimerID;
                pCtx->pData = pData;

                handle = m_pTimerEngine->AddTimer((PVOID)pCtx, millseconds, bRepeat);
                if (handle)
                {
                        m_mapTimerHandle[nTimerID] = handle;
                        return true;
                }
                else
                {
                        z3_free(pCtx);
                }
        }

        return false;
}

bool TimerObj::DeleteTimer(uint32_t nTimerID)
{
        HANDLE handle;

        handle = m_mapTimerHandle[nTimerID];

        // if key is not found, map shall insert along with the default value of HANDLE        
        if (handle != Z3_INVALID_TIMER_HANDLE)
        {
                assert(m_pTimerEngine != NULL);
                if (true == m_pTimerEngine->DeleteTimer(GetObjID(), handle))
                {
                        m_mapTimerHandle.erase(nTimerID);
                        return true;
                }
        }

        return false;
}

void TimerObj::OnTimer(uint32_t nTimerID, void *pData)
{
        TRACE_WARN("Default Timer function is invokded. You should override function: %s\r\n", __FUNCTION__);
        return;
}
