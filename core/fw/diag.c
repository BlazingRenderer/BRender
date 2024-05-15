/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: diag.c 1.1 1997/12/10 16:41:06 jon Exp $
 * $Locker: $
 *
 * Diagnostic handling support
 *
 * These routines are not called directly, but via macros. This is
 * to allow the underlying diagnostic mechanism to be radically changed
 * and allow various text scanning methods for message extraction
 *
 */
#include <stdarg.h>

#include "fw.h"


static char _diag_scratch[128];

void BR_RESIDENT_ENTRY BrFailure(const char *s, ...)
{
    va_list     args;
    static char failure_header[] = "Failure: ";

    BrStrCpy(_diag_scratch, failure_header);

    va_start(args, s);
    BrVSprintf(_diag_scratch + (BR_ASIZE(failure_header) - 1), s, args);
    va_end(args);

    if(fw.diag->failure == NULL)
        BrAbort();

    fw.diag->failure(_diag_scratch);
}

void BR_RESIDENT_ENTRY BrWarning(const char *s, ...)
{
    va_list     args;
    static char warning_header[] = "Warning: ";

    BrStrCpy(_diag_scratch, warning_header);

    va_start(args, s);
    BrVSprintf(_diag_scratch + (BR_ASIZE(warning_header) - 1), s, args);
    va_end(args);

    if(fw.diag->warning == NULL)
        BrAbort();

    fw.diag->warning(_diag_scratch);
}

void BR_RESIDENT_ENTRY BrFatal(const char *name, int line, char *s, ...)
{
    va_list args;
    int     n;

    n = BrSprintf(_diag_scratch, "FATAL %s:%d\n", name, line);
    va_start(args, s);
    BrVSprintf(_diag_scratch + n, s, args);
    va_end(args);

    if(fw.diag->failure == NULL)
        BrAbort();

    fw.diag->failure(_diag_scratch);
}

void BR_RESIDENT_ENTRY _BrAssert(const char *condition, const char *file, unsigned line)
{
    if(fw.diag->failure == NULL)
        BrAbort();

    BrSprintf(_diag_scratch, "ASSERTION FAILED %s:%d: \"%s\"\n", file, line, condition);
    fw.diag->failure(_diag_scratch);
}

void BR_RESIDENT_ENTRY _BrUAssert(const char *condition, const char *file, unsigned line)
{
    if(fw.diag->failure == NULL)
        BrAbort();

    BrSprintf(_diag_scratch, "ASSERTION FAILED %s:%d: \"%s\"\n", file, line, condition);
    fw.diag->failure(_diag_scratch);
}

void BR_PUBLIC_ENTRY BrDebugBreak(void)
{
#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__GNUC__)
    /*
     * __buildin_trap() must be conditional,
     * otherwise no code will be generated afterwards.
     */
    volatile int v = 1;
    if(v)
        __builtin_trap();
#else
    assert(false);
#endif
}
