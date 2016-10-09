#include "z3_common.hpp"
#include "z3_iocp_obj.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

IOCPObj::IOCPObj(HANDLE hIOCP, uint32_t nObjID)
        : AsyncObj(nObjID)
        , m_hIOCP(hIOCP)
{
        assert(m_hIOCP != NULL);
}

IOCPObj::~IOCPObj()
{
        LPZ3OVL pOvl;

        while(!m_lstIdle.empty())
        {
                pOvl = m_lstIdle.front();

                z3_free(pOvl);
                m_lstIdle.pop_front();
        }
}

LPZ3OVL IOCPObj::AllocZ3Ovl(HANDLE hFileHandle, ev_id_t evID, uint32_t millseconds)
{
        struct __timeb64 timeout;
        uint32_t seconds;       
        LPZ3OVL pOvl;

        /*Lock();*/
        if (!m_lstIdle.empty())
        {
                pOvl = m_lstIdle.front();
                m_lstIdle.pop_front();
        }
        else
                pOvl = (LPZ3OVL)z3_calloc(1, sizeof(Z3OVL));
        /*Unlock();*/

        if (!pOvl)
                return NULL;
        
        pOvl->file_handle = hFileHandle;
        pOvl->act_ovl.ev_id = evID;
        pOvl->timeout_ovl.ev_id = EV_TIMEOUT;

        pOvl->data = this;

        if (millseconds == INFINITE)
        {
                timeout.time = 0xBBF81E00;      // 100 年以后
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

void IOCPObj::FreeZ3Ovl(LPZ3OVL pOvl)
{
        /*Lock();*/
        if (m_lstIdle.size() > 3)
        {
                z3_free(pOvl);
        }
        else
        {
                memset(pOvl, 0, sizeof(Z3OVL));
                m_lstIdle.push_back(pOvl);
        }
        /*Unlock();*/
}

int IOCPObj::Start(void)
{
        BOOL    bOK;
        LPZ3OVL pZ3Ovl;

        // pZ3Ovl 内存的删除，由z3_engine负责
        pZ3Ovl = AllocZ3Ovl(NULL, EV_INSTANCE_START, INFINITE);
        assert(pZ3Ovl);

        // 加入z3_engine 中的PENGDING列表，同时完成超时侦测
        bOK = ::PostQueuedCompletionStatus(m_hIOCP, 0, GetObjID(), TIMEOUT_OVL_ADDR(pZ3Ovl));
        if (!bOK)
        {
                TRACE_ERROR("Failed to invoke function PostQueuedCompletionStatus in connector object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                FreeZ3Ovl(pZ3Ovl);
                return Z3_SYS_ERROR;
        }

        // 投递一个“空”事件（实际上是随便投递一个事件），让IOCP启动，自动开始Dispatch，对象运行；
        bOK = ::PostQueuedCompletionStatus(m_hIOCP, 0, GetObjID(), ACT_OVL_ADDR(pZ3Ovl));
        if (!bOK)
        {
                TRACE_ERROR("Failed to invoke function PostQueuedCompletionStatus in connector object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                FreeZ3Ovl(pZ3Ovl);
                return Z3_SYS_ERROR;
        }

        return 0;        
}