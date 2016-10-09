#include "z3_common.hpp"
#include "z3_msg.hpp"

using namespace Z3;

Msg::Msg(uint32_t nObjID)
        : AsyncObj(nObjID)
        , m_pBuffer(NULL)
        , m_nBufSize(0)
{
}

Msg::~Msg()
{
        Z3_FREE_POINTER(m_pBuffer);
}

char * Msg::ToString(unsigned int *pnSize)
{
        assert(m_pBuffer == NULL);

        if (ToStringImpl(&m_pBuffer, &m_nBufSize))
        {
                *pnSize = m_nBufSize;
                return m_pBuffer;
        }
        
        *pnSize = 0;

        return NULL;                
}