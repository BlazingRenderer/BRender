#include "fw.h"

br_uint_8 BR_RESIDENT_ENTRY BrLogSetLevel(br_uint_8 level)
{
    br_uint_8 old = fw.log_level;

    if(level > BR_LOG_TRACE)
        level = BR_LOG_TRACE;
    else if(level < BR_LOG_OFF)
        level = BR_LOG_OFF;

    fw.log_level = level;
    return old;
}

br_uint_8 BR_RESIDENT_ENTRY BrLogGetLevel(void)
{
    return fw.log_level;
}

void BR_RESIDENT_ENTRY BrLogV(br_uint_8 level, const char *component, const char *fmt, va_list ap)
{
    if(level > fw.log_level)
        return;

     fw.log->handler(level, component, fmt, ap);
}

void BR_RESIDENT_ENTRY BrLog(br_uint_8 level, const char *component, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    BrLogV(level, component, fmt, ap);
    va_end(ap);
}

void BR_RESIDENT_ENTRY BrLogTrace(const char *component, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    BrLogV(BR_LOG_TRACE, component, fmt, ap);
    va_end(ap);
}

void BR_RESIDENT_ENTRY BrLogDebug(const char *component, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    BrLogV(BR_LOG_DEBUG, component, fmt, ap);
    va_end(ap);
}

void BR_RESIDENT_ENTRY BrLogInfo(const char *component, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    BrLogV(BR_LOG_INFO, component, fmt, ap);
    va_end(ap);
}

void BR_RESIDENT_ENTRY BrLogWarn(const char *component, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    BrLogV(BR_LOG_WARN, component, fmt, ap);
    va_end(ap);
}

void BR_RESIDENT_ENTRY BrLogError(const char *component, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    BrLogV(BR_LOG_ERROR, component, fmt, ap);
    va_end(ap);
}