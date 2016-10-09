#include "z3_common.hpp"
#include "z3_rtsp_client.hpp"
#include "z3_rtsp_def.hpp"

using namespace Z3;

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

RtspClient::RtspClient(uint32_t nObjID)
        : Client(nObjID)
{
}

RtspClient::~RtspClient()
{  
        assert(m_lstSessions.empty());
}

//int RtspClient::AddSession(const char *pszHost, uint16_t nPort)
//{
//        RtspSession     *pSession;
//        HANDLE          hIOCP;
//
//        assert(Running());
//        hIOCP = GetIOCP();
//        assert(hIOCP);
//
//        pSession = new RtspSession(this, hIOCP);
//        pSession->AddRef();
//
//        if (!pSession->SetDestination(pszHost, nPort))
//                return Z3_ENOMEM;
//
//        return pSession->Start();
//}

void RtspClient::OnNotify(uint32_t nSessionState, void *pData)
{
        RtspSession *pSession;

        pSession = static_cast<RtspSession *>(pData);

        switch (nSessionState)
        {
        case RtspSession::STATE_CONNECT_TIMEOUT:
                OnConnectTimeout(pSession);
                break;
        default:
                break;
        }

        return;
}

void RtspClient::OnConnectTimeout(RtspSession *pSession)
{
        TRACE_INFO("Session 0x%p timeout\r\n", pSession);
        
        m_lstSessions.remove(pSession);
        Z3_OBJ_RELEASE(pSession);
}

int RtspClient::AddSession(const char *pszUrl)
{
        HANDLE          hIOCP;
        RtspSession     *pSession;
        int             nError = Z3_ERROR;

        assert(Running());
        hIOCP = GetIOCP();
        assert(hIOCP);

        pSession = new RtspSession(this, hIOCP);

        nError = pSession->SetRequestUrl(pszUrl);
        m_lstSessions.push_back(pSession);
        Z3_OBJ_ADDREF(pSession);

        if (nError == Z3_EOK)
                return pSession->Start();

        return nError;
}

void RtspClient::OnClientStart(void)
{
        // nothing;
}

void RtspClient::OnClientStop(void)
{
        RtspSession *pSession;

        while (!m_lstSessions.empty())
        {
                pSession = m_lstSessions.front();
                Z3_OBJ_RELEASE(pSession);

                m_lstSessions.pop_front();
        }
}
