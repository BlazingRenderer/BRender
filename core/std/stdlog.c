#include <stdio.h>
#include <brender.h>

static void BR_CALLBACK BrStdioLogProc(br_uint_8 level, const char *component, const char *fmt, va_list ap)
{
    const char *sLevel;

    switch(level) {
        case BR_LOG_TRACE:
            sLevel = "TRACE";
            break;
        case BR_LOG_DEBUG:
            sLevel = "DEBUG";
            break;
        case BR_LOG_INFO:
            sLevel = "INFO";
            break;
        case BR_LOG_WARN:
            sLevel = "WARN";
            break;
        case BR_LOG_ERROR:
            sLevel = "ERROR";
            break;
        default:
            sLevel = "UNKN";
    }

    (void)fprintf(stderr, "%s: %s: ", sLevel, component);
    (void)vfprintf(stderr, fmt, ap);
    (void)fputc('\n', stderr);
}

br_loghandler BrStdioLogHandler = {
    .identifier = "Stdio LogHandler",
    .handler    = BrStdioLogProc,
};

/*
 * Override default
 */
br_loghandler *BR_ASM_DATA _BrDefaultLogHandler = &BrStdioLogHandler;
