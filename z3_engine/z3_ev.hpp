#ifndef _Z3_EV_HPP_
#define _Z3_EV_HPP_

typedef enum _ev_id
{
        EV_UNKNOWN,
        EV_INSTANCE_START,
        EV_INSTANCE_STOP,
        EV_TIMEOUT,
        EV_READ,
        EV_WRITE,
        EV_CONNECT,
        EV_ACCEPT,
} ev_id_t;

typedef struct _Z3_EV_OVERLAPPED
{
        OVERLAPPED      ovl;
        ev_id_t         ev_id;
} Z3_EV_OVL;

typedef struct _Z3_OVERLAPPED
{
        Z3_EV_OVL               act_ovl;
        Z3_EV_OVL               timeout_ovl;
        HANDLE                  file_handle;
        struct __timeb64        timeout;    // timeout millseconds;
        SOCKADDR_IN             remote_addr;
        int                     remote_addr_size;
        void                    *data;
} Z3OVL, *LPZ3OVL;

typedef struct _Z3_EV
{
        ev_id_t id;
        void    *data;
} Z3EV, *LPZ3EV;

#define ACT_OVL_ADDR(z3ovl_pointer)                     ((LPOVERLAPPED)(z3ovl_pointer))
#define TIMEOUT_OVL_ADDR(z3ovl_pointer)                 (LPOVERLAPPED)((char *)z3ovl_pointer + sizeof(Z3_EV_OVL))
#define Z3OVL_ADDR_FROM_TIMEOUTOVL(timeout_ovl_pointer) (LPZ3OVL)((char *)timeout_ovl_pointer - sizeof(Z3_EV_OVL))
#define Z3OVL_ADDR_FROM_ACTOVL(act_ovl_pointer)         (LPZ3OVL)(act_ovl_pointer)
#define SOCK_ADDR_FROM_Z3OVL(z3ovl_pointer)             (z3ovl_pointer->remote_addr)
#define SOCK_ADDR_SIZE_FROM_Z3OVL(z3ovl_pointer)        (z3ovl_pointer->remote_addr_size)
#define GET_EV_ID(poverlapped)                          (((Z3_EV_OVL *)poverlapped)->ev_id)

#define CLEAN_ACT_OVL(z3ovl_pointer)            { memset(&z3ovl_pointer->act_ovl, 0, sizeof(Z3_EV_OVL)); }
#define CLEAN_TIMEOUT_OVL(z3ovl_pointer)        { memset(&z3ovl_pointer->timeout_ovl, 0, sizeof(Z3_EV_OVL)); }


#endif