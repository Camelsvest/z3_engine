#include "z3_memory_obj.hpp"
#include "z3_alloc.h"

using namespace Z3;

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