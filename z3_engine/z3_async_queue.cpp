#include "z3_common.hpp"
#include "z3_async_queue.hpp"
#include "z3_iocp_obj.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

template<class TYPE, class TYPE_ARG>
AsyncQueue<TYPE, TYPE_ARG>::AsyncQueue()
        : m_hEvent(NULL)
        , m_hMutex(NULL)
{
        m_hEvent = ::CreateEventA(NULL, TRUE, FALSE, NULL);
        assert(m_hEvent != NULL);

        m_hMutex = ::CreateMutexA(NULL, FALSE, NULL);
        assert(m_hMutex != NULL);
}

template<class TYPE, class TYPE_ARG>
AsyncQueue<TYPE, TYPE_ARG>::~AsyncQueue()
{
        ::CloseHandle(m_hMutex);
        ::CloseHandle(m_hEvent);
}

template<class TYPE, class TYPE_ARG>
void AsyncQueue<TYPE, TYPE_ARG>::Push(TYPE_ARG element)
{
        DWORD   dwResult;

        dwResult = ::WaitForSingleObject(m_hMutex, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);
        
        m_Queue.push(element);

        ::ReleaseMutex(m_hMutex);        
}

template<class TYPE, class TYPE_ARG>
bool AsyncQueue<TYPE, TYPE_ARG>::Pop(TYPE &element)
{
        DWORD   dwResult;
        bool    bOK = false;

        dwResult = ::WaitForSingleObject(m_hMutex, INFINITE);
        assert(dwResult == WAIT_OBJECT_0);
        if (!m_Queue.empty())
        {
                element = m_Queue.front();
                m_Queue.pop();

                bOK = true;
        }
        ::ReleaseMutex(m_hMutex);

        return bOK;
}

template<class TYPE, class TYPE_ARG>
bool AsyncQueue<TYPE, TYPE_ARG>::Signal(bool bOK)
{
        assert(m_hEvent);

        if (bOK)
                return (::SetEvent(m_hEvent) == TRUE);
        else
                return (::ResetEvent(m_hEvent) == TRUE);
}

template<class TYPE, class TYPE_ARG>
bool AsyncQueue<TYPE, TYPE_ARG>::WaitForEV(TYPE &element, uint32_t nTimeout/* millseconds*/)
{
        DWORD   dwResult;
        HANDLE  Handles[2];
        bool    bOk;

        assert(m_hMutex != NULL);
        assert(m_hEvent != NULL);

        Handles[0] = m_hMutex;
        Handles[1] = m_hEvent;
        bOk = false;

        dwResult = ::WaitForMultipleObjects(2, Handles, TRUE, nTimeout);        
        if (dwResult == WAIT_OBJECT_0)
        {
                if (!m_Queue.empty())
                {
                        element = m_Queue.front();
                        m_Queue.pop();

                        bOk = true;
                }
        
                if (m_Queue.empty())
                        ::ResetEvent(m_hEvent);

                ::ReleaseMutex(m_hMutex);
        }

        return bOk;
}