#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>

#include "vx_type.h"
#include "vx_log.h"



#define VX_LOG_MAX_LEN     256

typedef void (*vx_log_callback)(const char*, const char*, va_list);


static  VX_U32 vx_log_open = 0;
static VX_U32 vx_log_flag = 0;

// TODO: add log timing information and switch flag
static const char *msg_log_warning = "log message is long\n";
static const char *msg_log_nothing = "\n";



#define LINE_SZ 1024
void os_log(const char* tag, const char* msg, va_list list)
{
    char line[LINE_SZ] = {0};
    snprintf(line, sizeof(line), "%s: %s", tag, msg);
    vsyslog(LOG_INFO, line, list);
}

void os_err(const char* tag, const char* msg, va_list list)
{
    char line[LINE_SZ] = {0};
    snprintf(line, sizeof(line), "%s: %s", tag, msg);
    vsyslog(LOG_ERR, line, list);
}

static void __vx_log(vx_log_callback func, const char *tag, const char *fmt,
                      const char *fname, va_list args)
{
    char msg[VX_LOG_MAX_LEN + 1];
    char *tmp = msg;
    const char *buf = fmt;
    if (vx_log_open == 0)
    {
        vx_log_open  = 1;
        openlog("vx", LOG_PID | LOG_CONS | LOG_PERROR, LOG_USER);
    }
    size_t len_fmt  = strnlen(fmt, VX_LOG_MAX_LEN);
    size_t len_name = (fname) ? (strnlen(fname, VX_LOG_MAX_LEN)) : (0);
    size_t buf_left = VX_LOG_MAX_LEN;
    size_t len_all  = len_fmt + len_name;
    if (NULL == tag)
        tag = MODULE_TAG;
    if (len_name) {
        buf = msg;
        buf_left -= snprintf(msg, buf_left, "%s ", fname);
        tmp += len_name + 1;
    }
    if (len_all == 0) {
        buf = msg_log_nothing;
    } else if (len_all >= VX_LOG_MAX_LEN) {
        buf_left -= snprintf(tmp, buf_left, "%s", msg_log_warning);
        buf = msg;
    } else {
        snprintf(tmp, buf_left, "%s", fmt);
        if (fmt[len_fmt - 1] != '\n') {
            tmp[len_fmt]    = '\n';
            tmp[len_fmt + 1]  = '\0';
        }
        buf = msg;
    }
    func(tag, buf, args);
}

void _vx_log(const char *tag, const char *fmt, const char *fname, ...)
{
    va_list args;
    va_start(args, fname);
    __vx_log(os_log, tag, fmt, fname, args);
    va_end(args);
}

void _vx_err(const char *tag, const char *fmt, const char *fname, ...)
{
    va_list args;
    va_start(args, fname);
    __vx_log(os_err, tag, fmt, fname, args);
    va_end(args);
}

void vx_log_set_flag(VX_U32 flag)
{
    vx_log_flag = flag;
    return ;
}

VX_U32 vx_log_get_flag()
{
    return vx_log_flag;
}
