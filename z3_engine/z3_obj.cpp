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

AsyncObj::AsyncObj(uint32_t nObjID)
        : m_nObjID(nObjID)
        , m_nRefCount(0)
{
}

AsyncObj::~AsyncObj()
{
        assert(m_nRefCount == 0);
}

uint32_t AsyncObj::GetRefCount()
{
        uint32_t nCount;

        m_Lock.On();
        nCount = m_nRefCount;
        m_Lock.Off();

        return nCount;
}

uint32_t AsyncObj::AddRef()
{
        uint32_t nCount;

        m_Lock.On();
        nCount = ++m_nRefCount;
        m_Lock.Off();

        return nCount;
}

void AsyncObj::Release()
{
        uint32_t nCount;

        m_Lock.On();
        if (m_nRefCount > 0)
                --m_nRefCount;
        nCount = m_nRefCount;
        m_Lock.Off();

        if (nCount == 0)
                delete this;
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