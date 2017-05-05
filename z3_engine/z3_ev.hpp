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
        EV_OP_ADD,
        EV_OP_REMOVE,
        EV_NOTIFY
} ev_id_t;

typedef struct _Z3_EV_OVERLAPPED
{
        OVERLAPPED      ovl;
        ev_id_t         ev_id;
        HANDLE          iocp_handle;
        HANDLE          timer;
        void            *data;
} Z3_EV_OVL, *LPZ3_EV_OVL;

//typedef struct _Z3_OVERLAPPED
//{
//        Z3_EV_OVL               act_ovl;
//        Z3_EV_OVL               timeout_ovl;
//        HANDLE                  file_handle;
//        struct __timeb64        timeout;    // timeout millseconds;
//        SOCKADDR_IN             remote_addr;
//        int                     remote_addr_size;
//        void                    *data;
//} Z3OVL, *LPZ3OVL;

typedef struct _Z3_EV_ASYNCQUEUE_ITEM
{
        ev_id_t id;
        void    *data;
} Z3EV_ASYNCQUEUE_ITEM, *LPZ3EV_ASYNCQUEUE_ITEM;

typedef struct _Z3_EV_NOTIFY_ITEM
{
        ev_id_t         id;
        uint32_t        nErrorCode;
        void            *data;
} Z3EV_NOTIFY_ITEM, *LPZ3EV_NOTIFY_ITEM;

#define ACT_OVL_ADDR(z3ovl_pointer)                     ((LPOVERLAPPED)(&(z3ovl_pointer->ovl)))
//#define TIMEOUT_OVL_ADDR(z3ovl_pointer)                 (LPOVERLAPPED)((char *)z3ovl_pointer + sizeof(Z3_EV_OVL))
//#define Z3OVL_ADDR_FROM_TIMEOUTOVL(timeout_ovl_pointer) (LPZ3OVL)((char *)timeout_ovl_pointer - sizeof(Z3_EV_OVL))
//#define Z3OVL_ADDR_FROM_ACTOVL(act_ovl_pointer)         (LPZ3OVL)(act_ovl_pointer)
//#define SOCK_ADDR_FROM_Z3OVL(z3ovl_pointer)             (z3ovl_pointer->remote_addr)
//#define SOCK_ADDR_SIZE_FROM_Z3OVL(z3ovl_pointer)        (z3ovl_pointer->remote_addr_size)
#define GET_EV_ID(poverlapped)                          (((Z3_EV_OVL *)poverlapped)->ev_id)
#define GET_EV_DATA(poverlapped)                        (((Z3_EV_OVL *)poverlapped)->data)

#define CLEAN_ACT_OVL(z3ovl_pointer)            { memset(&z3ovl_pointer->act_ovl, 0, sizeof(Z3_EV_OVL)); }
//#define CLEAN_TIMEOUT_OVL(z3ovl_pointer)        { memset(&z3ovl_pointer->timeout_ovl, 0, sizeof(Z3_EV_OVL)); }

inline int interpret_ev_id(ev_id_t evID, char *pszBuf, unsigned int nLength)
{
        errno_t err;

        if (pszBuf == NULL)
                return 0;
        
        if (nLength <= 0)
                return 0;
        else if (nLength <= 32)
        {
                *pszBuf = '\0';
                return 0;
        }
        else
        {
                switch (evID)
                {
                case EV_UNKNOWN:
                        err = strcpy_s(pszBuf, nLength, "EV_UNKNOWN");
                        break;
                case EV_INSTANCE_START:
                        err = strcpy_s(pszBuf, nLength, "EV_INSTANCE_START");
                        break;
                case EV_INSTANCE_STOP:
                        err = strcpy_s(pszBuf, nLength, "EV_INSTANCE_STOP");
                        break;
                case EV_TIMEOUT:
                        err = strcpy_s(pszBuf, nLength, "EV_TIMEOUT");
                        break;
                case EV_READ:
                        err = strcpy_s(pszBuf, nLength, "EV_READ");
                        break;
                case EV_WRITE:
                        err = strcpy_s(pszBuf, nLength, "EV_WRITE");
                        break;
                case EV_CONNECT:
                        err = strcpy_s(pszBuf, nLength, "EV_CONNECT");
                        break;
                case EV_ACCEPT:
                        err = strcpy_s(pszBuf, nLength, "EV_ACCEPT");
                        break;
                case EV_OP_ADD:
                        err = strcpy_s(pszBuf, nLength, "EV_OP_ADD");
                        break;
                case EV_OP_REMOVE:
                        err = strcpy_s(pszBuf, nLength, "EV_OP_REMOVE");
                        break;
                case EV_NOTIFY:
                        err = strcpy_s(pszBuf, nLength, "EV_NOTIFY");
                        break;
                default:
                        assert(false);
                        break;
                }

                assert(err == 0);
                return strlen(pszBuf);
        }


}

#endif