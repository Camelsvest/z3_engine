#include "z3_common.hpp"
#include "z3_iocp_obj.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

IOCPObj::IOCPObj(HANDLE hIOCP, uint32_t nObjID)
        : TimerObj(nObjID)
        , m_hIOCP(hIOCP)
{
        assert(m_hIOCP != NULL);
}

IOCPObj::~IOCPObj()
{
}


inline int IOCPObj::PostCompletionStatus(LPOVERLAPPED pOvl)
{
        BOOL bOK;

        bOK = ::PostQueuedCompletionStatus(m_hIOCP, 0, GetObjID(), pOvl);
        if (!bOK)
        {
                TRACE_ERROR("Failed to invoke function PostQueuedCompletionStatus in IOCP object 0x%p, function %s, file %s, line %d\r\n",
                        this, __FUNCTION__, __FILE__, __LINE__);

                return Z3_SYS_ERROR;
        }

        return Z3_EOK;
}

int IOCPObj::Start(void)
{
        int             iRet;
        LPZ3_EV_OVL     pZ3Ovl;

        // pZ3Ovl �ڴ��ɾ������z3_engine����
        pZ3Ovl = AllocZ3Ovl(EV_INSTANCE_START, 0);
        assert(pZ3Ovl);

        TRACE_DETAIL("Allocating Z3Ovl(0x%p) to start IOCP object.\r\n", pZ3Ovl);

        // Ͷ��һ����EV_INSTANCE_START���¼�����IOCP�������Զ���ʼDispatch���������У�
        iRet = PostCompletionStatus(ACT_OVL_ADDR(pZ3Ovl));
        if (iRet != Z3_EOK)
        {
                FreeZ3Ovl(pZ3Ovl);
                TRACE_DETAIL("Free Z3Ovl(0x%p) because failing to post completion status.\r\n", pZ3Ovl);
        }

        return iRet;
}

int IOCPObj::Stop(void)
{
        int             iRet;
        LPZ3_EV_OVL     pZ3Ovl;

        // pZ3Ovl �ڴ��ɾ������z3_engine����
        pZ3Ovl = AllocZ3Ovl(EV_INSTANCE_STOP, 0);
        assert(pZ3Ovl);

        TRACE_DETAIL("Allocating Z3Ovl(0x%p) to start IOCP object.\r\n", pZ3Ovl);

        // Ͷ��һ����EV_INSTANCE_STOP���¼�����IOCP�������Զ���ʼDispatch���������У�
        iRet = PostCompletionStatus(ACT_OVL_ADDR(pZ3Ovl));
        if (iRet != Z3_EOK)
        {
                FreeZ3Ovl(pZ3Ovl);
                TRACE_DETAIL("Free Z3Ovl(0x%p) because failing to post completion status.\r\n", pZ3Ovl);
        }

        return iRet;
}

void IOCPObj::OnTimer(void *pData)
{
        LPZ3_EV_OVL     pZ3Ovl;

	pZ3Ovl = static_cast<LPZ3_EV_OVL>(pData);
        assert(pZ3Ovl != NULL);
	assert(pZ3Ovl->iocp_handle != INVALID_HANDLE_VALUE);

        /*
  	 * �������
	 * 1. �ɹ��ˣ�IO��ȡ����Z3_ENGINE����ӦIO�����жϴ���
	 * 2. ʧ���ˣ���ζ��IO���ڽ����У��޷�ȡ������ȻZ3_ENGINE���Լ�������IO�¼�
	 *
	 * ���Ǳ���TIMER�еĲ���ʱ�価���̣��󲿷ֹ�����Z3_ENGINE�д������
	 */
	::CancelIoEx(pZ3Ovl->iocp_handle, (LPOVERLAPPED)pZ3Ovl);

	return;
}

int IOCPObj::Run(ev_id_t evID, uint32_t nErrorCode, uint32_t nBytes, bool bExpired)
{
        int nResult = Z3_EOK;

        switch (evID)
        {
        case EV_INSTANCE_START:
                nResult = OnStart();
                break;

        case EV_INSTANCE_STOP:
                nResult = OnStop();
                break;

        default:
                nResult = OnEvCompleted(evID, nErrorCode, nBytes, bExpired);
                break;
        }

        return nResult;
}