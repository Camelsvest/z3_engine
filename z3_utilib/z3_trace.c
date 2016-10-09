#include <Windows.h>
#include <assert.h>
#include <process.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>

#include "z3_trace.h"
#include "z3_list.h"

#define MAX_DUMP_SIZE   512
#define MAX_LINE_LENGTH 1024

typedef struct _truck truck_t;
struct _truck
{
        HANDLE          thread;
        HANDLE          mutex;
        HANDLE          msg_event;
        HANDLE          idle_event;
        long            quit;
        z3_list_t       *string_list;

        unsigned long   inputted_bytes;
        unsigned long   outputted_bytes;
};
static truck_t  *G_TRUCK_INFO = NULL;

static void for_each(void *data_orig, void *data_custom)
{
        char *string;

        string = (char *)data_orig;

        assert(strlen(string) > 0);
        OutputDebugStringA(string);

        free(string);

        return;
}


static int shipping(const char *string, int size_with_null)
{
        z3_list_t *list;
        char    *msg;
        DWORD   result;
        HANDLE  handles[2];
        BOOL    hangry = FALSE, reset = FALSE;
        
        msg = (char *)malloc(size_with_null);
        assert(msg);
        memcpy(msg, string, size_with_null);

        handles[0] = G_TRUCK_INFO->mutex;
        handles[1] = G_TRUCK_INFO->idle_event;

        result = WaitForMultipleObjects(2, handles, TRUE, INFINITE);
        assert(result == WAIT_OBJECT_0);


        list = z3_list_add_tail(G_TRUCK_INFO->string_list, msg);
        if (!G_TRUCK_INFO->string_list)
                G_TRUCK_INFO->string_list = list;

        ReleaseMutex(G_TRUCK_INFO->mutex);
        
        SetEvent(G_TRUCK_INFO->msg_event);

        return size_with_null;
}

static unsigned __stdcall thread_func(void *args)
{
        truck_t *truck_info;
        DWORD   result;
        BOOL    succeed, quit;
        HANDLE  handle_array[2];

        truck_info = (truck_t *)args;
        assert(truck_info);

        handle_array[0] = truck_info->msg_event;
        handle_array[1] = truck_info->mutex;

        quit = FALSE;
        while (!quit)
        {
                result = WaitForMultipleObjects(2, handle_array, TRUE, 50);
                if (result == WAIT_TIMEOUT)
                {
                        result = WaitForSingleObject(truck_info->msg_event, 0);
                        if (result == WAIT_OBJECT_0)
                        {
                                succeed = ResetEvent(truck_info->idle_event);
                                assert(succeed);
                        }

                        // check whether we're quiting
                        result = WaitForSingleObject(truck_info->mutex, INFINITE);
                        assert(result == WAIT_OBJECT_0);
                        
                        quit = truck_info->quit;
                        succeed = ReleaseMutex(truck_info->mutex);
                        assert(succeed);
                        
                        continue;
                }
                
                assert(result == WAIT_OBJECT_0);
              
                z3_list_foreach(truck_info->string_list, for_each, NULL);
                z3_list_free(truck_info->string_list);
                truck_info->string_list = NULL;

                quit = truck_info->quit;

                succeed = ReleaseMutex(truck_info->mutex);
                assert(succeed);

                succeed = ResetEvent(truck_info->msg_event);
                assert(succeed);

                succeed = SetEvent(truck_info->idle_event);
                assert(succeed);
        }

        succeed = WaitForSingleObject(truck_info->mutex, 20);

        z3_list_foreach(truck_info->string_list, for_each, NULL);
        z3_list_free(truck_info->string_list);
        truck_info->string_list = NULL;

        succeed = ReleaseMutex(truck_info->mutex);
        assert(succeed);

        return 0;
}

static int start_truck(long level)
{
        unsigned int thread_id = 0;

        if (!G_TRUCK_INFO)
        {
                G_TRUCK_INFO = (truck_t *)calloc(1, sizeof(truck_t));
                assert(G_TRUCK_INFO);

                G_TRUCK_INFO->mutex = CreateMutex(NULL, FALSE, NULL);
                assert(G_TRUCK_INFO->mutex);

                G_TRUCK_INFO->msg_event = CreateEvent(NULL, TRUE, FALSE, NULL);
                assert(G_TRUCK_INFO->msg_event);

                G_TRUCK_INFO->idle_event = CreateEvent(NULL, TRUE, TRUE, NULL);
                assert(G_TRUCK_INFO->idle_event);

                G_TRUCK_INFO->inputted_bytes = 0;
                G_TRUCK_INFO->outputted_bytes = 0;

                G_TRUCK_INFO->thread = (HANDLE)_beginthreadex(NULL, 0, thread_func, G_TRUCK_INFO, 0, &thread_id);
                assert(G_TRUCK_INFO->thread != (HANDLE)-1L);

                return 0;
        }

        return -1;
}

static void stop_truck(void)
{
        DWORD   result;

        assert(G_TRUCK_INFO);

        result = WaitForSingleObject(G_TRUCK_INFO->mutex, INFINITE);
        assert(result == WAIT_OBJECT_0);

        G_TRUCK_INFO->quit = TRUE;
        ReleaseMutex(G_TRUCK_INFO->mutex);

        SetEvent(G_TRUCK_INFO->msg_event);      // notify to quit

        result = WaitForSingleObject(G_TRUCK_INFO->thread, INFINITE);
        assert(result == WAIT_OBJECT_0);

        CloseHandle(G_TRUCK_INFO->thread);
        CloseHandle(G_TRUCK_INFO->mutex);
        CloseHandle(G_TRUCK_INFO->msg_event);
        CloseHandle(G_TRUCK_INFO->idle_event);
        
        z3_list_free(G_TRUCK_INFO->string_list);
        free(G_TRUCK_INFO);

        return;
}

typedef struct _trace_info trace_info_t;
struct _trace_info
{
        HANDLE  mutex;
        long    level;

        long    is_first_call;
        LARGE_INTEGER   tick_frequency;
        LARGE_INTEGER   epoch_offset;
};

static trace_info_t *G_TRACE_INFO = NULL;

int TRACE_INIT(long nLevel)
{
        if (!G_TRACE_INFO)
        {
                G_TRACE_INFO = (trace_info_t *)calloc(1, sizeof(trace_info_t));
                assert(G_TRACE_INFO);
                
                G_TRACE_INFO->mutex = CreateMutex(NULL, FALSE, NULL);
                assert(G_TRACE_INFO->mutex != NULL);

                G_TRACE_INFO->level = nLevel;
                G_TRACE_INFO->is_first_call = 1;
        }

        start_truck(nLevel);

        return 0;
}

void TRACE_UNINIT()
{
        stop_truck();

        if (G_TRACE_INFO)
        {
                CloseHandle(G_TRACE_INFO->mutex);
                free(G_TRACE_INFO);
        }
}

static int gettimeofday(struct timeval* tp, int *tz)
{
        DWORD           result;
        LARGE_INTEGER   tickNow;

        QueryPerformanceCounter(&tickNow);

        assert(G_TRACE_INFO);
        result = WaitForSingleObject(G_TRACE_INFO->mutex, INFINITE);
        assert(result == WAIT_OBJECT_0);

        if (G_TRACE_INFO->is_first_call)
        {
                // For our first call, use "ftime()", so that we get a time with a proper epoch.
                // For subsequent calls, use "QueryPerformanceCount()", because it's more fine-grain.

                struct __timeb32 tb;
                _ftime32_s(&tb);
                tp->tv_sec = tb.time;
                tp->tv_usec = 1000*tb.millitm;

                // Also get our counter frequency:
                QueryPerformanceFrequency(&G_TRACE_INFO->tick_frequency);

                // And compute an offset to add to subsequent counter times, so we get a proper epoch:
                G_TRACE_INFO->epoch_offset.QuadPart
                        = tb.time*G_TRACE_INFO->tick_frequency.QuadPart + (tb.millitm*G_TRACE_INFO->tick_frequency.QuadPart)/1000 - tickNow.QuadPart;

                G_TRACE_INFO->is_first_call = 0; // for next time
        }
        else
        {
                // Adjust our counter time so that we get a proper epoch:
                tickNow.QuadPart += G_TRACE_INFO->epoch_offset.QuadPart;

                tp->tv_sec = (long) (tickNow.QuadPart / G_TRACE_INFO->tick_frequency.QuadPart);
                tp->tv_usec = (long) (((tickNow.QuadPart % G_TRACE_INFO->tick_frequency.QuadPart) * 1000000L) / G_TRACE_INFO->tick_frequency.QuadPart);
        }

        ReleaseMutex(G_TRACE_INFO->mutex);

        return 0;
}

static int output_prefix(char *buf, unsigned int buf_size, const char *token)
{
        int             bytes;
        struct timeval  timestamp;	
        struct tm       tmTime;                

        gettimeofday(&timestamp, NULL);
        _localtime32_s(&tmTime, (__time32_t *)&timestamp.tv_sec);

        assert(buf_size >= 64);
        bytes = sprintf_s(buf, buf_size, "[%02d:%02d:%02d.%06d] Thread:0x%08X, %6s: ", 
                tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec, timestamp.tv_usec, GetCurrentThreadId(), token);

        return bytes;
}

static long get_trace_level()
{
        long    current;
        DWORD   result;

        result = WaitForSingleObject(G_TRACE_INFO->mutex, INFINITE);
        assert(result == WAIT_OBJECT_0);

        current = G_TRACE_INFO->level;
        ReleaseMutex(G_TRACE_INFO->mutex);

        return current;
}

static int trace_string(const char *token, const char *string)
{
        char buf[MAX_LINE_LENGTH];
        int  bytes, offset;

        offset = output_prefix(buf, sizeof(buf), token);
        assert(offset > 0);

        strcat_s(buf, sizeof(buf), string);
        offset = strlen(buf);
        assert(offset > 0);

        bytes = shipping(buf, offset + 1);
        assert(bytes == offset + 1);

        return bytes;
}

typedef struct _dump_buf dump_buf_t;
struct _dump_buf
{
        char            *buf;
        unsigned        buf_size;
        char            *token;
};

static void trace_each_line(void *data_orig, void *data_custom)
{
        dump_buf_t      *ctx;
        char            *string;
        char            line[MAX_LINE_LENGTH];

        string = (char *)data_orig;
        ctx = (dump_buf_t *)data_custom;

        assert(strlen(string) <= MAX_LINE_LENGTH);

        output_prefix(line, sizeof(line), ctx->token);
        strcat_s(ctx->buf, ctx->buf_size, line);

        strcat_s(ctx->buf, ctx->buf_size, (char *)data_orig);
        free(string);

        return;
}


static int trace_string_list(const char *token, z3_list_t *string_list)
{
        dump_buf_t ctx;
        unsigned int length;
        int bytes;

        ctx.buf = (char *)calloc(1, 8192);
        assert(ctx.buf);

        ctx.token = (char *)token;
        ctx.buf_size = 8192;

        z3_list_foreach(string_list, trace_each_line, (void *)&ctx);
        length = strlen(ctx.buf);

        bytes = shipping(ctx.buf, length+1);

        free(ctx.buf);

        return bytes;
}

static int trace(long level, const char *token, const char *format, va_list args)
{
        long    current;
        char    buf[MAX_LINE_LENGTH];
        int     bytes, offset;

        current = get_trace_level();
        bytes = 0;
        if (current >= level)
        {
                offset = output_prefix(buf, sizeof(buf), token);
                assert(offset > 0);

                bytes = vsprintf_s(buf + offset, sizeof(buf) - offset, format, args);
                assert(bytes > 0);

                offset += bytes;
              
                bytes = shipping(buf, offset + 1);
                assert(bytes == offset + 1);
        }

        return bytes;
}

static z3_list_t* list_add_string(z3_list_t *list, const char *string, unsigned int length_with_null)
{
        char            *line;

        line = (char *)calloc(1, length_with_null);
        assert(line);
        strcpy_s(line, length_with_null, string);

        return z3_list_add_tail(list, line);
}

static int dump(long level, const char *token, const unsigned char *buf, unsigned int size)
{
        long            current;
        int             bytes, i;
        unsigned char   ch, *pos;
        char            line[MAX_LINE_LENGTH];
        z3_list_t       *list = NULL;

        current = get_trace_level();
        if (current < level)
                return 0;

        if (size > MAX_DUMP_SIZE)
        {
                bytes = sprintf_s(line, sizeof(line), "buffer size is too big(%d bytes), now only display first 512 bytes.\r\n", size);
                assert(bytes > 0);
                size = MAX_DUMP_SIZE;

                /*trace_string(token, line);*/
                list = list_add_string(list, line, bytes + 1);
        }

        strcpy_s(line, sizeof(line), "Dump Binary:\r\n");        
        bytes = strlen(line);
        assert(bytes > 0);
        /*trace_string(token, line);*/
        list = list_add_string(list, line, bytes + 1);

        i = 0;
        pos = (unsigned char *)buf;

        // print the address we are pulling from
        bytes = sprintf_s(line, sizeof(line), "%08X | ", (int)buf);
        assert(bytes > 0);

        while (size-- > 0)
        {
                // print each char
                bytes += sprintf_s(line + bytes, sizeof(line) - bytes, "%02X ", *buf++);

                if (!(++i % 16) || (size == 0 && i % 16))
                {
			// if we come to the end of a line...
       
			// if this is the last line, print some fillers.
			if (size == 0)
			{
				while (i++ % 16)
				{ 
					bytes += sprintf_s(line + bytes, sizeof(line) - bytes, "__ ");
				}
			}

                        bytes += sprintf_s(line + bytes, sizeof(line) - bytes, "| ");

			while (pos < buf) // print the character version
			{  
				ch = *pos++;
                                bytes += sprintf_s(line + bytes, sizeof(line) - bytes, "%c", (ch < 33 || ch == 255) ? 0x2E : ch);				
			}

			// If we are not on the last line, prefix the next line with the address.
			if (size > 0)
			{
                                bytes += sprintf_s(line + bytes, sizeof(line) - bytes, "\r\n");
                                /*trace_string(token, line);*/
                                list = list_add_string(list, line, bytes + 1);

                                bytes = sprintf_s(line, sizeof(line), "%08X | ", (int)buf);
			}                        
                }                
        }

        bytes += sprintf_s(line + bytes, sizeof(line) - bytes, "\r\n\r\n");
        /*trace_string(token, line);*/
        list = list_add_string(list, line, bytes + 1);

        bytes = trace_string_list(token, list);
        z3_list_free(list);

        return bytes;
}

int TRACE_DETAIL(const char *format, ...)
{
        int     bytes;
        va_list args;

        va_start(args, format);
        bytes = trace(LOG_DETAIL, "DETAIL", format, args);
        va_end(args);

        return bytes;
}

int TRACE_DEBUG(const char *format, ...)
{
        int     bytes;
        va_list args;

        va_start(args, format);
        bytes = trace(LOG_DEBUG, "DEBUG", format, args);
        va_end(args);

        return bytes;
}

int TRACE_INFO(const char *format, ...)
{
        int     bytes;
        va_list args;

        va_start(args, format);
        bytes = trace(LOG_INFO, "INFO", format, args);
        va_end(args);

        return bytes;
}

int TRACE_NOTICE(const char *format, ...)
{
        int     bytes;
        va_list args;

        va_start(args, format);
        bytes = trace(LOG_NOTICE, "NOTICE", format, args);
        va_end(args);

        return bytes;
}

int TRACE_WARN(const char *format, ...)
{
        int     bytes;
        va_list args;

        va_start(args, format);
        bytes = trace(LOG_WARN, "WARN", format, args);
        va_end(args);

        return bytes;
}

int TRACE_ERROR(const char *format, ...)
{
        int     bytes;
        va_list args;

        va_start(args, format);
        bytes = trace(LOG_ERROR, "ERROR", format, args);
        va_end(args);

        return bytes;
}

int TRACE_CRIT(const char *format, ...)
{
        int     bytes;
        va_list args;

        va_start(args, format);
        bytes = trace(LOG_CRIT, "CRIT", format, args);
        va_end(args);

        return bytes;
}

int TRACE_ALERT(const char *format, ...)
{
        int     bytes;
        va_list args;

        va_start(args, format);
        bytes = trace(LOG_ALERT, "ALERT", format, args);
        va_end(args);

        return bytes;
}

int TRACE_FATAL(const char *format, ...)
{
        int     bytes;
        va_list args;

        va_start(args, format);
        bytes = trace(LOG_FATAL, "FATAL", format, args);
        va_end(args);

        return bytes;
}

int TRACE_DUMP(long level, const char *buf, unsigned int size)
{
        int     bytes;
        char    token[16];

        switch(level)
        {
        case LOG_DETAIL:
                strcpy_s(token, sizeof(token), "DETAIL");
                break;
        case LOG_DEBUG:
                strcpy_s(token, sizeof(token), "DEBUG");
                break;
        case LOG_INFO:
                strcpy_s(token, sizeof(token), "INFO");
                break;
        case LOG_NOTICE:
                strcpy_s(token, sizeof(token), "NOTICE");
                break;
        case LOG_WARN:
                strcpy_s(token, sizeof(token), "WARN");
                break;
        case LOG_ERROR:
                strcpy_s(token, sizeof(token), "ERROR");
                break;
        case LOG_CRIT:
                strcpy_s(token, sizeof(token), "CRIT");
                break;
        case LOG_ALERT:
                strcpy_s(token, sizeof(token), "ALERT");
                break;
        case LOG_FATAL:
                strcpy_s(token, sizeof(token), "FATAL");
                break;
        default:
                assert(FALSE);
        }

        bytes = dump(level, token, buf, size);

        return bytes;
}