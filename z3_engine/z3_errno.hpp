#ifndef _Z3_ERRNO_HPP_
#define _Z3_ERRNO_HPP_

enum _Z3_ERRNO
{
        Z3_EOK  = 0,
        EBASE   = 0x399B6000,
        Z3_ERROR,
        Z3_ENOMEM,
        Z3_EINVAL,
        Z3_EINTR,
        //...

        EWSABASE = 0x40000000,
        ESYSBASE = 0x60000000
};

#define Z3_WSA_ERROR    (EWSABASE + WSAGetLastError())
#define Z3_SYS_ERROR    (ESYSBASE + GetLastError())


#endif