// z3_rtsp_client_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "z3_common.hpp"
#include "z3_rtsp_client.hpp"
#include "z3_media_ports.hpp"

#ifdef Z3_MEM_DEBUG
#define new z3_debug_new
#endif

using namespace Z3;

int _tmain(int argc, _TCHAR* argv[])
{
        RtspClient      *pClient;
        int             nError;
        WSADATA         wsaData;

        nError = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (nError != NO_ERROR) {
                printf("Error at WSAStartup()\n");
                return 1;
        }

        TRACE_INIT(LOG_DEBUG);
        z3_alloc_init();

        pClient = new RtspClient();
        Z3_OBJ_ADDREF(pClient);

        nError = pClient->Start();
        assert(!nError);

        nError = pClient->AddSession("rtsp://192.168.101.100/zoo.mkv");
        if (nError != Z3_EOK)
                TRACE_ERROR("Failed to add URL rtsp://192.168.101.100/zoo.mkv, error code %d", nError);

        printf("Press any key to quit...");

        getchar();

        pClient->Stop();
        TRACE_DEBUG("Client stopped\r\n");

        Z3_OBJ_RELEASE(pClient);        
        MediaPorts::Destroy();
        z3_alloc_uninit();

        TRACE_DEBUG("Now stop tracing...\r\n");
        TRACE_UNINIT();

        WSACleanup();

	return 0;
}

