#include "z3_common.hpp"
#include "z3_ev.hpp"

LPZ3_EV_OVL AllocZ3Ovl(HANDLE hFileHandle, ev_id_t evID, uint32_t millseconds, void *data)
{
        struct __timeb64 timeout;
        uint32_t seconds;
        LPZ3_EV_OVL pOvl;

        pOvl = (LPZ3_EV_OVL)z3_calloc(1, sizeof(Z3_EV_OVL));
        if (!pOvl)
                return NULL;

        if (evID == EV_TIMEOUT)
                pOvl->handle.timer_id = (uint32_t)hFileHandle;
        else
                pOvl->handle.file_handle = hFileHandle;

        pOvl->ev_id = evID;
        pOvl->data = data;

        if (millseconds == INFINITE)
        {
                timeout.time = 0xBBF81E00;      // 100 ÄêÒÔºó
                timeout.millitm = 0x3E7;        // 999 millseconds
        }
        else
        {
                _ftime64_s(&timeout);

                seconds = millseconds / 1000;
                timeout.time += seconds;

                millseconds -= seconds * 1000;
                timeout.millitm += millseconds;
                while (timeout.millitm >= 1000)
                {
                        timeout.time += 1;
                        timeout.millitm -= 1000;
                }
        }

        pOvl->timeout = timeout;

        return pOvl;
}

void FreeZ3Ovl(LPZ3_EV_OVL pOvl)
{
        z3_free(pOvl);
}