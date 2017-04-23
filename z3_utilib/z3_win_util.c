#include <windows.h>
#include <strsafe.h>

#include "z3_win_util.h"
#include "z3_alloc.h"

int retrieve_msg_string(uint32_t nWinCode, char **ppOutString)
{
        // Retrieve the system error message for the last-error code
        LPVOID lpMsgBuf;
        int    nLength;

        FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                nWinCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf,
                0, NULL);

#if defined(_UNICODE)
        WideCharToMultiByte(....)
#else
        nLength = strlen((char *)lpMsgBuf);
        if (nLength > 0)
        {
                *ppOutString = z3_malloc(nLength + 1);
                strcpy_s(*ppOutString, nLength + 1, (char *)lpMsgBuf);
        }
#endif

        LocalFree(lpMsgBuf);

        return nLength;
}
