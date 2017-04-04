#ifndef _Z3_COMMON_HPP_
#define _Z3_COMMON_HPP_

#include <WinSock2.h>
#include <Ws2tcpip.h>
//#include <Mstcpip.h>
#include <Mswsock.h>
#include <Windows.h>
#include <process.h>
#include <tchar.h>

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>

#include <list>
#include <queue>
#include <map>
#include <algorithm>

#include "z3_trace.h"
#include "z3_alloc.h"
#include "z3_errno.hpp"
#include "z3_ev.hpp"
#include "z3_obj_ids.hpp"

#define INVALID_OBJ_ID          0xFFFFFFFF
#define Z3_CLOSE_HANDLE(p)      if (p) { ::CloseHandle(p); p = NULL; }
#define Z3_FREE_POINTER(p)      if (p) { z3_free(p); p = NULL; }
#define Z3_CLOSE_SOCKET(p)      if (p != INVALID_SOCKET) { ::closesocket(p); p = NULL; }

#define Z3_DELETE_OBJ(p)        \
        if (p)                  \
        {                       \
                delete p;       \
                p = NULL;       \
        }

#define Z3_DELETE_OBJ_ARRAY(p)  \
        if (p)                  \
        {                       \
                delete []p;     \
                p = NULL;       \
        }

#define SOCKET_CONNECTING_TIMEOUT       7000       // millseconds
#define SOCKET_READ_TIMEOUT             7000
#define SOCKET_WRITE_TIMEOUT            3000

#define THREAD_NUM_FACTOR_PER_CPU       2

#define SOCKET_SEND_BUFSIZE             4096
#define SOCKET_RECV_BUFSIZE             4096

#endif