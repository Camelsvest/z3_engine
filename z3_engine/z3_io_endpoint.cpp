#include "z3_common.hpp"
#include "z3_io_endpoint.hpp"

using namespace Z3;

IOEndpoint::IOEndpoint(HANDLE hIOCP, uint32_t nObjID)
        : IOCPObj(hIOCP, nObjID)
        , m_hFileHandle(INVALID_HANDLE_VALUE)
{

}

IOEndpoint::~IOEndpoint()
{

}

inline void IOEndpoint::SetFileHandle(HANDLE hFileHandle)
{
        //Lock();

        assert(m_hFileHandle == INVALID_HANDLE_VALUE);

        m_hFileHandle = hFileHandle;

        //Unlock();
}

HANDLE IOEndpoint::GetFileHandle()
{
        return m_hFileHandle;
}

LPZ3_EV_OVL IOEndpoint::AllocZ3Ovl(ev_id_t evID, uint32_t millseconds)
{
        LPZ3_EV_OVL pOvl;

        pOvl = (LPZ3_EV_OVL)z3_calloc(1, sizeof(Z3_EV_OVL));
        if (!pOvl)
                return NULL;

        if (millseconds > 0 && millseconds < INFINITE)
        {
                if (!AddTimer(&pOvl->timer, pOvl, millseconds))
                {
                        z3_free(pOvl);
                        return NULL;
                }
        }
        else
                pOvl->timer = INVALID_HANDLE_VALUE;

        assert(m_hFileHandle != INVALID_HANDLE_VALUE);
        pOvl->iocp_handle = m_hFileHandle;

        pOvl->ev_id = evID;
        pOvl->data = this;

        return pOvl;
}

void IOEndpoint::FreeZ3Ovl(LPZ3_EV_OVL pOvl)
{
        z3_free(pOvl);
}
