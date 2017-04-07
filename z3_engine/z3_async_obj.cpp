#include "z3_common.hpp"
#include "z3_async_obj.hpp"

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

