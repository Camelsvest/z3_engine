#include "stdafx.h"
#include "z3_common.hpp"
#include "z3_client_test.h"

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

using namespace Z3;

TestClient::TestClient()
{

}

TestClient::~TestClient()
{

}

void TestClient::OnClientStart()
{

}

void TestClient::OnClientStop()
{

}

Connection* TestClient::AddConnection(const char *pszHost, uint16_t port)
{
        Connection *pConn;
        HANDLE hIOCP;

        hIOCP = GetIOCP();

        pConn = new Connection(hIOCP);
        Z3_OBJ_ADDREF(pConn);
        if (!pConn->SetDestination(pszHost, port))
        {
                Z3_OBJ_RELEASE(pConn);
                return NULL;
        }

        pConn->Start();

        return pConn;

}

void TestClient::OnNotify(uint32_t nSessionState, void *pData)
{

}