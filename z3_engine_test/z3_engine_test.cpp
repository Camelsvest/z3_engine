// z3_engine_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "z3_common.hpp"
#include "z3_connection.h"
#include "z3_client_test.h"

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

using namespace Z3;

int main()
{
        TestClient      *pClient;
        Connection      *pConn;
        int             nError;
        WSADATA         wsaData;

        nError = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (nError != NO_ERROR) {
                printf("Error at WSAStartup()\n");
                return 1;
        }

        TRACE_INIT(LOG_DETAIL);
        z3_alloc_init();

        pClient = new TestClient();
        Z3_OBJ_ADDREF(pClient);

        nError = pClient->Start();
        assert(!nError);

        pConn = pClient->AddConnection("192.168.1.118", 4554);

        printf("Press any key to quit ...");
        getchar();

        Z3_OBJ_RELEASE(pConn);
        Z3_OBJ_RELEASE(pClient);

        z3_alloc_uninit();
        TRACE_UNINIT();

        WSACleanup();

        return 0;
}

