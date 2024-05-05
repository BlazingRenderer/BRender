#ifndef _BRLOG_H_
#define _BRLOG_H_

#if !defined(__H2INC__)
#include <stdarg.h>
#endif

#include "compiler.h"

enum {
    BR_LOG_OFF   = 0,
    BR_LOG_ERROR = 1,
    BR_LOG_WARN  = 2,
    BR_LOG_INFO  = 3,
    BR_LOG_DEBUG = 4,
    BR_LOG_TRACE = 5,
};

/*
 * Instance of a log handler.
 */
#if !defined(__H2INC__)
typedef void BR_CALLBACK br_log_cbfn(void *user, br_uint_8 level, const char *component, const char *fmt, va_list ap);
#else
typedef void BR_CALLBACK br_log_cbfn(void *user, br_uint_8 level, const char *component, const char *fmt);
#endif

typedef struct br_loghandler {
    const char  *identifier;
    br_log_cbfn *handler;
    void        *user;
} br_loghandler;

#endif /* _BRLOG_H_ */
