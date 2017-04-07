#include "z3_ref_obj.hpp"

using namespace Z3;

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
