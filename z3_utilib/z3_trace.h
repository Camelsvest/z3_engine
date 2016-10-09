#ifndef _Z3_TRACE_H_
#define _Z3_TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

enum _LOG_LEVEL
{
        LOG_FATAL,
        LOG_ALERT,
        LOG_CRIT,
        LOG_ERROR,
        LOG_WARN,
        LOG_NOTICE,
        LOG_INFO,
        LOG_DEBUG,
        LOG_DETAIL
};

int     TRACE_INIT(long  nLevel);
void    TRACE_UNINIT();

int     TRACE_DETAIL(const char *format, ...);
int     TRACE_DEBUG(const char *format, ...);
int     TRACE_INFO(const char *format, ...);
int     TRACE_NOTICE(const char *format, ...);
int     TRACE_WARN(const char *format, ...);
int     TRACE_ERROR(const char *format, ...);
int     TRACE_CRIT(const char *format, ...);
int     TRACE_ALERT(const char *format, ...);
int     TRACE_FATAL(const char *format, ...);
int     TRACE_DUMP(long level, const char *buf, unsigned int size);

#define TRACE_ENTER_FUNCTION       TRACE_DETAIL("Exter function %s\r\n", __FUNCTION__)
#define TRACE_EXIT_FUNCTION        TRACE_DETAIL("Exit function %s\r\n", __FUNCTION__)

#ifdef __cplusplus
}
#endif

#endif