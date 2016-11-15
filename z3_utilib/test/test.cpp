// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include "z3_alloc.h"
#include "z3_trace.h"

#define MAX_BUF_SIZE	256
#define MAX_THREADS	20

unsigned int getline(char **buf, size_t *buf_size, FILE *fp)
{
        char *pos = *buf;
        size_t bytes = 0;

        while (bytes < *buf_size)
        {
                pos[bytes] = fgetc(fp);
                if (pos[bytes] == EOF || pos[bytes] == '\n')
                        break;

                bytes++;
        }

        return bytes;
}

unsigned _stdcall thread_func(void *args)
{
        FILE	*fp;
        char	*buf, *dupbuf;
        int	read_bytes;
        size_t	buf_size;
        int     index = 0;
        int     thread_index = (int)args;

        fp = fopen("./sample.txt", "r");
        if (!fp)
        {
                printf("Failed to open file ./sample.txt\r\n");
                goto end2;
        }

        buf_size = MAX_BUF_SIZE;
        buf = (char *)z3_malloc(buf_size);
        if (!buf)
        {
                TRACE_ERROR("Failed to allocate %d bytes memory\r\n", MAX_BUF_SIZE);
                goto end1;
        }

        read_bytes = getline(&buf, &buf_size, fp);
        while (read_bytes > 0)
        {
                assert(read_bytes < buf_size);
                buf[read_bytes] = '\0';

                dupbuf = z3_strdup(buf);
                TRACE_INFO("%s", dupbuf);
                z3_free(dupbuf);

                //++index;
                //buf = (char *)z3_realloc(buf, buf_size + index);
                read_bytes = getline(&buf, &buf_size, fp);
        }

end1:
        fclose(fp);
        z3_free(buf);

end2:
        TRACE_NOTICE("Thread %lu quit, thread_index = %d\r\n", GetCurrentThreadId(), thread_index);
        return 0;
}
int main()
{
        HANDLE  	thread[MAX_THREADS];
        int		index = 0;
        DWORD           result;

        TRACE_INIT(LOG_NOTICE);
        z3_alloc_init();

        while (index < MAX_THREADS)
        {
                thread[index] = (HANDLE)_beginthreadex(NULL, 0, thread_func, (void *)index, 0, NULL);
                ++index;
        }

        printf("Press any key to quit");
        getchar();

        index = 0;
        while (index < MAX_THREADS)
        {
                result = WaitForSingleObject(thread[index], INFINITE);
                assert(result == WAIT_OBJECT_0);

                TRACE_NOTICE("Thread exited, index = %d\r\n", index);

                ++index;
        }

        z3_alloc_uninit();
        TRACE_UNINIT();

        return 0;
}

