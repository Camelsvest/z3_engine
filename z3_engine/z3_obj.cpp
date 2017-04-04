#include "z3_common.hpp"
#include "z3_obj.hpp"

using namespace Z3;

Lock::Lock()
        : m_hMutex(NULL)
{
}

Lock::~Lock()
{
        Z3_CLOSE_HANDLE(m_hMutex);
}

bool Lock::On()
{
        DWORD dwResult;

        if (m_hMutex == NULL)
        {
                m_hMutex = ::CreateMutexA(NULL, FALSE, NULL);
                assert(m_hMutex);
        }

        dwResult = ::WaitForSingleObject(m_hMutex, INFINITE);
        return (dwResult == WAIT_OBJECT_0);
}

void Lock::Off()
{
        assert (m_hMutex);
        ::ReleaseMutex(m_hMutex);
}

MemoryObject::MemoryObject()
{
}

MemoryObject::~MemoryObject()
{
}

void* MemoryObject::operator new(size_t nSize, char *pszFilename, unsigned int nLine)
{
#ifndef _DEBUG
        return malloc(nSize);
#else
        return _z3_malloc(nSize, pszFilename, nLine);
#endif
}

void MemoryObject::operator delete(void *p, char *pszFilename, unsigned int nLine)
{
        MemoryObject::operator delete(p);
}

void* MemoryObject::operator new[](size_t nSize, char *pszFilename, unsigned int nLine)
{
#ifndef _DEBUG
        return malloc(nSize);
#else
        return _z3_malloc(nSize, pszFilename, nLine);
#endif
}

void MemoryObject::operator delete[](void *p, char *pszFilename, unsigned int nLine)
{
#ifndef _DEBUG
        free(p);
#else
        _z3_free(p);
#endif
}

void* MemoryObject::operator new(size_t nSize)
{
#ifndef _DEBUG
        return malloc(nSize);
#else
        return z3_malloc(nSize);
#endif
}

void MemoryObject::operator delete(void *p)
{
#ifndef _DEBUG
        free(p);
#else
        z3_free(p);
#endif
}

void* MemoryObject::operator new[](size_t nSize)
{
#ifndef _DEBUG
        return malloc(nSize);
#else
        return z3_malloc(nSize);
#endif
}

void MemoryObject::operator delete[](void *p)
{
#ifndef _DEBUG
        free(p);
#else
        _z3_free(p);
#endif
}

RefObj::RefObj()
        : m_nRefCount(0)
{
}

RefObj::~RefObj()
{

}

uint32_t RefObj::GetRefCount()
{
        return m_nRefCount;
}

uint32_t RefObj::AddRef()
{
        return ++m_nRefCount;
}

void RefObj::Release(bool bFree /* = true*/)
{
        if (m_nRefCount > 0)
                --m_nRefCount;

        if (bFree && m_nRefCount == 0)
                delete this;
}

TimerEngine*    TimerEngine::m_pInstance = NULL;
HANDLE          TimerEngine::m_hMutex = NULL;
HANDLE          TimerEngine::m_hTimerQueue = NULL;

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

#ifndef _DEBUG
        free(pCtx);
#else
        _z3_free(pCtx);
#endif

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
        CloseHandle(hCompletionEvent);

        if (dwResult == WAIT_OBJECT_0 && bOK)
        {
                return true;
        }
             
        return false;
}

AsyncObj::AsyncObj(uint32_t nObjID)
        : m_nObjID(nObjID)
{
}

AsyncObj::~AsyncObj()
{
}

uint32_t AsyncObj::GetRefCount()
{
        uint32_t nCount;

        m_Lock.On();
        nCount = RefObj::GetRefCount();
        m_Lock.Off();

        return nCount;
}

uint32_t AsyncObj::AddRef()
{
        uint32_t nCount;

        m_Lock.On();
        nCount = RefObj::AddRef();
        m_Lock.Off();

        return nCount;
}

void AsyncObj::Release()
{
        uint32_t nCount;

        m_Lock.On();

        RefObj::Release(false);
        nCount = RefObj::GetRefCount();

        m_Lock.Off();

        if (nCount == 0)
                delete this;
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

#ifndef _DEBUG
        pCtx = (timer_ctx_t *)calloc(nSize);
#else
        pCtx = (timer_ctx_t *)_z3_calloc(sizeof(timer_ctx_t), 1, __FILE__, __LINE__);
#endif
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
#ifndef _DEBUG
                        free(pCtx);
#else
                        _z3_free(pCtx);
#endif
                }
        }

        return false;
}

bool TimerObj::DeleteTimer(uint32_t nTimerID)
{
        HANDLE handle;

        handle = m_mapTimerHandle[nTimerID];

        // if key is not found, map shall insert along with the default value of HANDLE        
        if (handle != 0)
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

void* operator new(size_t size, char *filename, unsigned int line)
{
        return ::_z3_malloc(size, filename, line);
}

void operator delete(void* p, char *filename, unsigned int line)
{
        _z3_free(p);
}

void* operator new[](size_t size, char *filename, unsigned int line)
{
        return operator new(size, filename, line);
}

void operator delete[](void* p, char *filename, unsigned int line)
{
        return operator delete(p, filename, line);
}